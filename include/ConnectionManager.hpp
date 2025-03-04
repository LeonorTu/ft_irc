#pragma once

#include <string>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <memory>
#include <Client.hpp>
#include <ClientIndex.hpp>
#include <SocketManager.hpp>
#include <EventLoop.hpp>

class ConnectionManager
{
public:
    ConnectionManager(ClientIndex &_clients, SocketManager &_socketManager, EventLoop &_EventLoop);
    ~ConnectionManager();

    void handleNewClient();
    void disconnectClient(Client *client);
    void recieveData(int clientFd);
    void extractFullMessages(Client *client, std::string &messageBuffer);
    void handleOversized(Client *client, std::string &messageBuffer);

private:
    ClientIndex &_clients;
    SocketManager &_socketManager;
    EventLoop &_EventLoop;
};
