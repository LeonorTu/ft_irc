#pragma once

#include <string>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <memory>
#include <Client.hpp>
#include <ClientIndex.hpp>
#include <SocketManager.hpp>
#include <EventLoop.hpp>
#include <CommandProcessor.hpp>

class ConnectionManager
{
public:
    ConnectionManager(SocketManager &socketManager, EventLoop &EventLoop, ClientIndex &clients);
    ~ConnectionManager();

    int handleNewClient();
    void disconnectClient(Client &client);
    void recieveData(int clientFd);
    void disconnectAllClients();

private:
    ClientIndex &_clients;
    SocketManager &_socketManager;
    EventLoop &_EventLoop;
    CommandProcessor _commandProcessor;

    void extractFullMessages(Client &client, std::string &messageBuffer);
    void handleOversized(Client &client, std::string &messageBuffer);
};
