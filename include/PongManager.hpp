#pragma once

#include <responses.hpp>
#include <ClientIndex.hpp>
#include <Client.hpp>

class ConnectionManager;
class PongManager
{
public:
    PongManager();
    ~PongManager();

    bool checkPingTimeouts(int timeoutMs, Client &client);
    void sendPingToClient(Client &client);
    void sendPingToAllClients(ClientIndex &clients);
    void checkAllPingTimeouts(int timeoutMs, ClientIndex &_clients, ConnectionManager &connManager);
    void handlePongFromClient(const std::string &token, Client &client);

}; 