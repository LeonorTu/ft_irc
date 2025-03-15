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
    void start();
    void loop();
    void shutdown();

    // getters
    static Server &getInstance();
    const int getServerFD() const;
    const bool getIsPaused() const;
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
    static Server *_instance;
    std::unique_ptr<ClientIndex> _clients;
    std::unique_ptr<ChannelManager> _channels;
    std::unique_ptr<SocketManager> _socketManager;
    std::unique_ptr<EventLoop> _eventLoop;
    std::unique_ptr<ConnectionManager> _connectionManager;
    static void signalHandler(int signum);
    const std::string _password;

    // server info
    const std::string _createdTime;

    volatile sig_atomic_t _running;
    volatile sig_atomic_t _paused;
    int _serverFD;
};
