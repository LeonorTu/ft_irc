#pragma once

#include <string>
#include <iostream>
#include <cstring>
#include <csignal>
#include <memory>

class SocketManager;
class EventLoop;
class ConnectionManager;
class ClientIndex;
class ChannelManager;

class Server
{
public:
    Server();
    ~Server();
    void start(std::string password);
    void loop();
    void shutdown();

    // getters
    static Server &getInstance();
    int getServerFD() const;
    bool getIsPaused() const;
    SocketManager &getSocketManager();
    EventLoop &getEventLoop();
    ClientIndex &getClients();
    ChannelManager &getChannels();
    ConnectionManager &getConnectionManager();
    const std::string &getPassword();
    const std::string &getCreatedTime();

    void pause();
    void resume();

private:
    int _serverFd;
    std::string _password;
    static Server *_instance;
    volatile sig_atomic_t _running;
    volatile sig_atomic_t _paused;
    std::unique_ptr<ClientIndex> _clients;
    std::unique_ptr<ChannelManager> _channels;
    std::unique_ptr<SocketManager> _socketManager;
    std::unique_ptr<EventLoop> _eventLoop;
    std::unique_ptr<ConnectionManager> _connectionManager;
    static void signalHandler(int signum);

    // server info
    const std::string _createdTime;
};
