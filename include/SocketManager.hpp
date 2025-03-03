#pragma once

#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>
#include <iostream>
#include <cstring>

class SocketManager
{
public:
    SocketManager(int port);
    ~SocketManager();

    int initialize();
    void closeAllConnections();
    int acceptConnection(sockaddr_in *clientAddr);
    void closeConnection(int fd);

private:
    int _serverFd;
    int _port;
    sockaddr_in _serverAddress;
};
