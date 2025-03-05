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

class Server
{
public:
    Server();
    ~Server();
    void start();
    void loop();
    void shutdown();

    // getters
    const int getServerFD() const;
    const bool getIsPaused() const;
    SocketManager &getSocketManager();
    EventLoop &getEventLoop();
    ClientIndex &getClients();
    ConnectionManager &getConnectionManager();

    // ChannelManager* getChannelManager();
    // setters
    static void setInstance(Server *server);

    void pause();
    void resume();

private:
    static Server *instance;
    std::unique_ptr<ClientIndex> _clients;
    std::unique_ptr<SocketManager> _socketManager;
    std::unique_ptr<EventLoop> _eventLoop;
    std::unique_ptr<ConnectionManager> _connectionManager;
    static void signalHandler(int signum);

    // server info
    std::string _createdTime;

    volatile sig_atomic_t running;
    volatile sig_atomic_t paused;
    int serverFD;

    void sendWelcome(int clientFd);
};
