#include "server.hpp"
#include "Client.hpp"
#include <common.hpp>

Server::Server() : port(SERVER_PORT), serverFD(-1)
{
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
    pollFDs.push_back(nextPollable(serverFD, POLLIN));
    bind(serverFD, (struct sockaddr *)&serverAddress, sizeof(serverAddress));
    listen(serverFD, -1);
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
        for (pollfd pfd : pollFDs) {
            if (pfd.fd != serverFD && pfd.revents & POLLIN) {
                Client *client = clients.at(pfd.fd);
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
    std::cout << "New client connected. Socket: " << clientFD << std::endl;
    std::cout << "Client's IP: " << ipAddress << std::endl;
}

void Server::parseMessage(Client *from)
{
    char buffer[BUFFER_SIZE] = {0};
    int bytesRead = recv(from->getFd(), buffer, sizeof(buffer), 0);
    if (bytesRead <= 0) {
        // Client disconnected or error
        std::cout << "Client disconnected: " << from->getFd() << std::endl;
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
