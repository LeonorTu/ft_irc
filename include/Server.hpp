#pragma once

#include <string>
#include <iostream>
#include <cstring>
#include <csignal>
#include <memory>
#include <chrono>

class SocketManager;
class EventLoop;
class ConnectionManager;
class ClientIndex;
class ChannelManager;
class PongManager;

class Server
{
public:
    Server(int port, std::string password, bool startBlocking = true);
    ~Server() noexcept;
    void loop();
    void shutdown();

    // getters
    static Server &getInstance();
    int getServerFD() const;
    SocketManager &getSocketManager();
    EventLoop &getEventLoop();
    ClientIndex &getClients();
    ChannelManager &getChannels();
    ConnectionManager &getConnectionManager();
    const std::string &getPassword();
    const std::string &getCreatedTime();
    PongManager &getPongManager();

    void pause();
    void resume();

    volatile sig_atomic_t _running;
    volatile sig_atomic_t _paused;

private:
    int _serverFd;
    int _port;
    std::string _password;
    static Server *_instance;
    std::unique_ptr<ClientIndex> _clients;
    std::unique_ptr<ChannelManager> _channels;
    std::unique_ptr<SocketManager> _socketManager;
    std::unique_ptr<EventLoop> _eventLoop;
    std::unique_ptr<PongManager> _PongManager;
    std::unique_ptr<ConnectionManager> _connectionManager;

    static void signalHandler(int signum);
    void pingSchedule(int64_t &last_ping);
    // void sendPingToInactivityClients(int timeoutMs, const int pingTimeout);
    // server info
    const std::string _createdTime;
};
