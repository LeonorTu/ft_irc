#pragma once

#include <string>
#include <unordered_map>
#include <netinet/in.h>
#include <unistd.h>
#include <iostream>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <cstring>
#include <sstream>
#include <chrono>
#include <csignal>
#include <memory>

#include <SocketManager.hpp>
#include <EventLoop.hpp>

class ClientIndex;

class Server
{
public:
    Server();
    ~Server();
    void start();
    void loop();
    void stop();

    // getters
    const int getServerFD() const;
    const int getPort() const;
    ClientIndex *getClients();
    SocketManager &getSocketManager();
    EventLoop &getEventLoop();
    // ChannelManager* getChannelManager();
    // ClientIndex* getClients();
    // setters
    static void setInstance(Server *server);

    bool paused;
    void pause();
    void resume();

private:
    std::unique_ptr<SocketManager> _socketManager;
    std::unique_ptr<EventLoop> _eventLoop;
    static Server *instance;
    static void signalHandler(int signum);
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
    ClientIndex *clients;

    void handleNewClient();
    void removeClient(int fd);
    void sendWelcome(int clientFD);
    void parseMessage(std::string msg);
    std::string recieveMessage(int fd);
    void cleanup();
};
