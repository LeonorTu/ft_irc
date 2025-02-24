#include "server.hpp"
#include "Client.hpp"
#include <common.hpp>
#include <sstream>
#include <fcntl.h>
#include <chrono>

Server::Server()
    : port(SERVER_PORT), serverFD(-1), serverName(SERVER_NAME), networkName(NETWORK_NAME),
      serverVersion(SERVER_VERSION), userModes(USER_MODES), channelModes(CHANNEL_MODES)
{
    // get current time for server start time with chrono
    auto now = std::chrono::system_clock::now();
    std::time_t now_time_t = std::chrono::system_clock::to_time_t(now);
    char buffer[80];
    std::strftime(buffer, sizeof(buffer), "%a %d %b %H:%M:%S %Y", std::localtime(&now_time_t));
    createdTime = std::string(buffer);

    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(port);
    serverAddress.sin_addr.s_addr = INADDR_ANY;
}

Server::~Server()
{
    close(serverFD);
    std::cout << "closed " << serverFD << std::endl;
}

pollfd nextPollable(int fd, short requested_ev)
{
    pollfd pollable;

    pollable.fd = fd;
    pollable.events = requested_ev;
    return pollable;
}

void Server::start()
{
    serverFD = socket(AF_INET, SOCK_STREAM, 0);
    if (serverFD < 0) {
        std::cout << "failed to start the server" << std::endl;
        cleanup();
    }
    fcntl(serverFD, F_SETFL, O_NONBLOCK);
    pollFDs.push_back(nextPollable(serverFD, POLLIN));
    bind(serverFD, (struct sockaddr *)&serverAddress, sizeof(serverAddress));
    listen(serverFD, 10);
    running = true;
    loop();
}

void Server::loop()
{
    while (running) {
        int ready = poll(pollFDs.data(), pollFDs.size(), -1);
        if (ready < 0) {
            std::cerr << "poll failed" << std::endl;
            continue;
        }
        if (pollFDs[0].revents & POLLIN) {
            handleNewClient(serverFD);
        }
        for (size_t i = 1; i < pollFDs.size(); ++i) {
            int fd = pollFDs[i].fd;
            if (pollFDs[i].revents & POLLIN) {
                Client *client = clients[fd];
                parseMessage(client);
            }
        }
    }
}

void Server::stop()
{
    running = false;
    cleanup();
}

const int Server::getServerFD() const
{
    return this->serverFD;
}

const int Server::getPort() const
{
    return this->port;
}

void Server::handleNewClient(int clientSocket)
{
    sockaddr_in clientAddr;
    socklen_t clientLen = sizeof(clientAddr);

    // accept new client connection
    int clientFD = accept(serverFD, (struct sockaddr *)&clientAddr, &clientLen);
    if (clientFD < 0) {
        std::cerr << "Failed to accept connection" << std::endl;
    }
    // get client's IP and add new Client to the map and polling list
    std::string ipAddress = inet_ntoa(clientAddr.sin_addr);
    clients[clientFD] = new Client(clientFD, ipAddress);
    pollFDs.emplace_back(nextPollable(clientFD, POLLIN));
    clients[clientFD]->setPollIndex(pollFDs.size() - 1);
    std::cout << "New client connected. Socket: " << clientFD << std::endl;
    sendWelcome(clientFD);
    std::cout << "Client's IP: " << ipAddress << std::endl;
}

void Server::removeClient(Client *client)
{
    int removedFD = client->getFd();
    size_t removedIndex = client->getPollIndex();

    // swap removed client to the back and update the pollindex of the other client that got swapped
    if (clients.size() > 1) {
        std::swap(pollFDs[removedIndex], pollFDs.back());
        clients[pollFDs[removedIndex].fd]->setPollIndex(removedIndex);
    }
    // remove client from map and vector
    clients.erase(removedFD);
    pollFDs.pop_back();

    close(removedFD);
    delete client;
}

// ":<source> <command/REPL> <parameters> <crlf>"
void Server::sendWelcome(int clientFD)
{
    // 001 RPL_WELCOME
    std::stringstream welcome;
    welcome << ":" << serverName << " 001 " << clientFD << " :Welcome to " << networkName << " Network, " << clientFD
            << "\r\n";
    send(clientFD, welcome.str().c_str(), welcome.str().size(), 0);
    // 002 RPL_YOURHOST
    std::stringstream yourHost;
    yourHost << ":" << serverName << " 002 " << clientFD << " :Your host is " << serverName << ", running version "
             << serverVersion << "\r\n";
    send(clientFD, yourHost.str().c_str(), yourHost.str().size(), 0);
    // 003 RPL_CREATED
    std::stringstream created;
    created << ":" << serverName << " 003 " << clientFD << " :This server was created " << createdTime << "\r\n";
    send(clientFD, created.str().c_str(), created.str().size(), 0);
    // 004 RPL_MYINFO
    std::stringstream myInfo;
    myInfo << ":" << serverName << " 004 " << clientFD << " " << serverName << " " << serverVersion << " " << userModes
           << " " << channelModes << "\r\n";
    send(clientFD, myInfo.str().c_str(), myInfo.str().size(), 0);
}

void Server::parseMessage(Client *from)
{
    char buffer[BUFFER_SIZE] = {0};
    int bytesRead = recv(from->getFd(), buffer, sizeof(buffer), 0);
    if (bytesRead <= 0) {
        // Client disconnected or error
        std::cout << "Client disconnected: " << from->getFd() << std::endl;
        // remove client from map and vector
        removeClient(from);
    }
    else {
        std::cout << "Message from " << from->getIP() << ": " << buffer << std::endl;
        std::string message(buffer);
        if (message.find("quit") != std::string::npos) {
            stop();
        }
    }
}

void Server::cleanup()
{
    for (std::pair client : clients) {
        close(client.first);
        delete (client.second);
    }
    clients.clear();
    pollFDs.clear();
    if (serverFD >= 0) {
        close(serverFD);
    }
    std::cout << "Server shutdown complete" << std::endl;
}
