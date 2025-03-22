#pragma once

#include <string>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <memory>
#include <Client.hpp>
#include <ClientIndex.hpp>
#include <SocketManager.hpp>
#include <EventLoop.hpp>
#include <MessageParser.hpp>
#include <PongManager.hpp>

class ConnectionManager
{
public:
    ConnectionManager(SocketManager &socketManager, EventLoop &EventLoop, ClientIndex &clients);
    ~ConnectionManager();

    void handleNewClient();
    void disconnectClient(Client &client, const std::string &reason);
    void recieveData(int clientFd);
    std::vector<Client *> &getDisconnectedClients();
    void markClientForDisconnection(Client &client);
    void rmDisconnectedClients();

    void cleanUp();

    private : ClientIndex &_clients;
    SocketManager &_socketManager;
    EventLoop &_EventLoop;
    std::vector<Client *> _clientsToDisconnect;

    void extractFullMessages(Client &client, std::string &messageBuffer);
    void handleOversized(Client &client, std::string &messageBuffer);

    void deleteClient(Client &client);
};
