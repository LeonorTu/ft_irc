#pragma once

#include <responses.hpp>
#include <CommandProcessor.hpp>
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

    void sendPingToAllClients(ClientIndex &clients);
    void checkAllPingTimeouts(int timeoutMs, ClientIndex &_clients, ConnectionManager &connManager);

private:
    std::unordered_map<std::string, std::chrono::steady_clock::time_point> _pingTokens;
    bool validateCommand(const CommandProcessor::CommandContext &ctx, const std::string &expected);
    bool validateParams(const CommandProcessor::CommandContext &ctx);

    void addPingToken(const std::string &token);
    void sendPongResponse(const CommandProcessor::CommandContext &ctx);
    bool rejectPingBeforeRegistered(const CommandProcessor::CommandContext &ctx);
    bool EmptyToken(const CommandProcessor::CommandContext &ctx);
    bool MoreThanOneToken(const CommandProcessor::CommandContext &ctx);
    // bool onlyColon(const CommandProcessor::CommandContext &ctx);

    void handlePongFromClient(const std::string &token);
    void sendPingToClient(Client &client);
    bool checkPingTimeouts(int timeoutMs);
}; // Added the semicolon here