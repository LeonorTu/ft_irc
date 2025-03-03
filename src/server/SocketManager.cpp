#include <SocketManager.hpp>

SocketManager::SocketManager(int port)
    : _port(port)
    , _serverFd(-1)
{
    _serverAddress.sin_port = htons(_port);
    _serverAddress.sin_family = AF_INET;
    _serverAddress.sin_addr.s_addr = INADDR_ANY;
}

SocketManager::~SocketManager()
{
    close(_serverFd);
}

int SocketManager::initialize()
{
    _serverFd = socket(AF_INET, SOCK_STREAM, 0);
    if (_serverFd < 0) {
        std::cerr << "Failed to create socket" << std::endl;
        return -1;
    }

    // Set socket options to reuse address (prevents "Address already in use" errors)
    int opt = 1;
    if (setsockopt(_serverFd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        std::cerr << "Failed to set socket options" << std::endl;
        close(_serverFd);
        return -1;
    }

    fcntl(_serverFd, F_SETFL, O_NONBLOCK);

    if (bind(_serverFd, (struct sockaddr *)&_serverAddress, sizeof(_serverAddress)) < 0) {
        std::cerr << "Failed to bind socket" << std::endl;
        close(_serverFd);
        return -1;
    }

    if (listen(_serverFd, SOMAXCONN) < 0) {
        std::cerr << "Failed to listen on socket" << std::endl;
        close(_serverFd);
        return -1;
    }

    return _serverFd;
}

void SocketManager::closeAllConnections()
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
        if (errno != EAGAIN && errno != EWOULDBLOCK) {
            std::cerr << "Failed to accept connection: " << strerror(errno) << std::endl;
        }
        return -1;
    }
    fcntl(clientFd, F_SETFL, O_NONBLOCK);
    return clientFd;
}

void SocketManager::closeConnection(int fd)
{
    close(fd);
}
