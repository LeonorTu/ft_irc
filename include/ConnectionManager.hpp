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
#include <ChannelManager.hpp>

class ConnectionManager
{
public:
    ConnectionManager(SocketManager &socketManager, EventLoop &EventLoop, ClientIndex &clients,
                      ChannelManager &channels);
    ~ConnectionManager();

    void handleNewClient();
    void disconnectClient(Client &client, const std::string &reason);
    void receiveData(int clientFd);
    std::vector<Client *> &getDisconnectedClients();
    void markClientForDisconnection(Client &client);
    void rmDisconnectedClients();

    void cleanUp();

private:
    ClientIndex &_clients;
    SocketManager &_socketManager;
    EventLoop &_EventLoop;
    ChannelManager &_channels;
    std::vector<Client *> _clientsToDisconnect;

    void extractFullMessages(Client &client, std::string &messageBuffer);
    void truncateAndProcessMessage(Client &client, std::string &message);

    void deleteClient(Client &client);
};
