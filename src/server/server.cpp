#include <server.hpp>
#include <Client.hpp>
#include <message.hpp>
#include <common.hpp>

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

    m_epoll_fd = epoll_create1(0);
    if (m_epoll_fd == -1) {
        std::cerr << "epoll create error" << std::endl;
    }
}

Server::~Server()
{
    cleanup();
}

void Server::start()
{
    serverFD = socket(AF_INET, SOCK_STREAM, 0);
    if (serverFD < 0) {
        std::cout << "failed to start the server" << std::endl;
        cleanup();
    }
    fcntl(serverFD, F_SETFL, O_NONBLOCK);
    bind(serverFD, (struct sockaddr *)&serverAddress, sizeof(serverAddress));
    listen(serverFD, 10);
    running = true;
    addPoll(serverFD, EPOLLIN);
    loop();
}

void Server::loop()
{
    while (running) {
        epoll_event events[EPOLL_MAX_EVENTS] = {0};
        int nfds = epoll_wait(m_epoll_fd, events, EPOLL_MAX_EVENTS, 100);
        if (nfds < 0) {
            std::cerr << "epoll failed" << std::endl;
            continue;
        }
        for (int i = 0; i < nfds; i++) {
            if (events[i].data.fd == serverFD)
                handleNewClient();
            else
                parseMessage(events[i].data.fd);
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

void Server::handleNewClient()
{
    sockaddr_in clientAddr;
    socklen_t clientLen = sizeof(clientAddr);

    // accept new client connection
    int clientFD = accept(serverFD, (struct sockaddr *)&clientAddr, &clientLen);
    if (clientFD < 0) {
        std::cerr << "Failed to accept connection" << std::endl;
    }
    // get client's IP and add new Client to the map and poll list
    std::string ipAddress = inet_ntoa(clientAddr.sin_addr);
    clients[clientFD] = new Client(clientFD, ipAddress);
    addPoll(clientFD, EPOLLIN);
    std::cout << "New client connected. Socket: " << clientFD << std::endl;
    sendWelcome(clientFD);
    std::cout << "Client's IP: " << ipAddress << std::endl;
}

void Server::removeClient(int fd)
{
    Client *client = clients[fd];
    removePoll(fd);
    clients.erase(fd);
    close(fd);
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

void Server::parseMessage(int fd)
{
    char buffer[MSG_BUFFER_SIZE] = {0};
    int bytesRead = recv(fd, buffer, sizeof(buffer), 0);
    if (bytesRead <= 0) {
        // Client disconnected or error
        std::cout << "Client disconnected: " << fd << std::endl;
        // remove client from map and vector
        removeClient(fd);
    }
    else {
        std::string rawMessage(buffer);
        std::cout << "Raw message: " << rawMessage << std::endl;
        message parsedMessage(rawMessage); // Use the new message struct!

        std::cout << "Parsed command: " << parsedMessage.command << std::endl;
        std::string message(buffer);
        if (message.find("quit") != std::string::npos) {
            stop();
        }
        else if (parsedMessage.command == "PRIVMSG") {
            std::cout << parsedMessage.prefix << " sent message to " << parsedMessage.parameters[0] << ": "
                      << parsedMessage.parameters[1] << std::endl;
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
    if (serverFD >= 0) {
        close(serverFD);
    }
    if (m_epoll_fd >= 0) {
        close(m_epoll_fd);
    }
    std::cout << "Server shutdown complete" << std::endl;
}

void Server::addPoll(int fd, uint32_t event)
{
    epoll_event ev;
    ev.data.fd = fd;
    ev.events = event;
    if (epoll_ctl(m_epoll_fd, EPOLL_CTL_ADD, fd, &ev) == -1) {
        std::cerr << "Failed to add fd to epoll: " << strerror(errno) << std::endl;
    }
}

void Server::removePoll(int fd)
{
    if (epoll_ctl(m_epoll_fd, EPOLL_CTL_DEL, fd, NULL) == -1) {
        std::cerr << "Failed to remove fd from epoll: " << strerror(errno) << std::endl;
    }
}
