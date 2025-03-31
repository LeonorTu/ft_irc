#include <SocketManager.hpp>
#include <Error.hpp>

SocketManager::SocketManager(int port)
    : _serverFd(-1)
    , _port(port)
{
    _serverAddress.sin_port = htons(_port);
    _serverAddress.sin_family = AF_INET;
    _serverAddress.sin_addr.s_addr = INADDR_ANY;
}

SocketManager::~SocketManager()
{
    closeServerSocket();
}

int SocketManager::initialize()
{
    _serverFd = socket(AF_INET, SOCK_STREAM, 0);
    if (_serverFd < 0) {
        throw SocketError("Failed to create socket");
    }

    // Set socket options to reuse address (prevents "Address already in use" errors)
    int opt = 1;
    if (setsockopt(_serverFd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        close(_serverFd);
        throw SocketError("Failed to set socket options");
    }

    fcntl(_serverFd, F_SETFL, O_NONBLOCK);

    if (bind(_serverFd, (struct sockaddr *)&_serverAddress, sizeof(_serverAddress)) < 0) {
        close(_serverFd);
        throw SocketError("Failed to bind socket");
    }

    if (listen(_serverFd, SOMAXCONN) < 0) {
        close(_serverFd);
        throw SocketError("Failed to listen on socket");
    }

    return _serverFd;
}

void SocketManager::closeServerSocket()
{
    if (_serverFd >= 0) {
        close(_serverFd);
        _serverFd = -1;
    }
}

int SocketManager::acceptConnection(sockaddr_in *clientAddr)
{
    sockaddr_in addr;
    socklen_t addrLen = sizeof(addr);
    sockaddr_in *addrPtr;
    if (clientAddr == nullptr)
        addrPtr = &addr;
    else
        addrPtr = clientAddr;
    int clientFd = accept(_serverFd, (struct sockaddr *)addrPtr, &addrLen);
    if (clientFd < 0) {
        throw SocketError("Failed to accept connection: " + std::string(strerror(errno)));
    }
    fcntl(clientFd, F_SETFL, O_NONBLOCK);
    return clientFd;
}

void SocketManager::closeConnection(int fd)
{
    if (fd >= 0)
        close(fd);
}
