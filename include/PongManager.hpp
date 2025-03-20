#pragma once

#include <responses.hpp>
#include <ClientIndex.hpp>
#include <ConnectionManager.hpp>
#include <Client.hpp>
#include <Channel.hpp>

class PongManager
{
public:
    PongManager();
    ~PongManager();

    bool checkPingTimeouts(int timeoutMs);
    void sendPingToClient(Client &client);
    void sendPingToAllClients(ClientIndex &clients);
    void checkAllPingTimeouts(int timeoutMs, ClientIndex &_clients, ConnectionManager &connManager);

private:
    std::unordered_map<std::string, std::chrono::steady_clock::time_point> _pingTokens;

    void addPingToken(const std::string &token);
    void handlePongFromClient(const std::string &token);

}; 