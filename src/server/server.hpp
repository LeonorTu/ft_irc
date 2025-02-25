#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <poll.h>
#include <iostream>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <cstring>

#include <common.hpp>

class Client;
class Server {
public:
    Server();
    ~Server();
    void start();
    void loop();
    void stop();
    void broadcastMessage(const std::string &message);

    // getters
    const int getServerFD() const;
    const int getPort() const;

private:
    // server info
    const std::string serverName;
    const std::string networkName;
    const std::string serverVersion;
    std::string createdTime;
    const std::string userModes;
    const std::string channelModes;

    bool running;
    int serverFD;
    int port;
    sockaddr_in serverAddress;
    std::unordered_map<int, Client *> clients;
    int m_epoll_fd;

    void handleNewClient();
    void removeClient(int fd);
    void sendWelcome(int clientFD);
    void parseMessage(int fd);
    void cleanup();

    void addPoll(int fd, uint32_t event);
    void removePoll(int fd);
};
