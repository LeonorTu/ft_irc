#pragma once

#include <responses.hpp>
#include <ClientIndex.hpp>
#include <ConnectionManager.hpp>
#include <Client.hpp>
#include <Channel.hpp>

class PingPongManager
{
public:
    PingPongManager();
    ~PingPongManager();

    void ping(const CommandProcessor::CommandContext &ctx);
    void pong(const CommandProcessor::CommandContext &ctx);

    bool checkPingTimeouts(int timeoutMs);
    void sendPingToClient(Client &client);
    void sendPingToAllClients(ClientIndex &clients);
    void checkAllPingTimeouts(int timeoutMs, ClientIndex &_clients, ConnectionManager &connManager);
    std::unordered_map<std::string, std::chrono::steady_clock::time_point> getPingTokens();

private:
    std::unordered_map<std::string, std::chrono::steady_clock::time_point> _pingTokens;

    void addPingToken(const std::string &token);
    void sendPongResponse(const CommandProcessor::CommandContext &ctx);


    void handlePongFromClient(const std::string &token);
    
    
}; // Added the semicolon here