#include <PingPongManager.hpp>

PingPongManager::PingPongManager() {};
PingPongManager::~PingPongManager() {};

bool PingPongManager::rejectPingBeforeRegistered(const CommandProcessor::CommandContext &ctx)
{
    if (!ctx.isRegistered) {
        std::cout << "Not registered client trying to Ping" << std::endl;
        return true;
    }
    return false;
}

bool PingPongManager::EmptyToken(const CommandProcessor::CommandContext &ctx)
{
    if (ctx.params.size() == 0) {
        sendToClient(ctx.clientFd, ERR_NEEDMOREPARAMS(ctx.nickname, ctx.command));
        std::cout << "Empty tokens from Client" << std::endl;
        return true;
    }
    return false;
}

bool PingPongManager::MoreThanOneToken(const CommandProcessor::CommandContext &ctx)
{
    // for (size_t i = 0; i < ctx.params.size(); ++i) {
    //     std::cout << "Param " << i << ": \"" << ctx.params[i] << "\"" << std::endl;
    // }
    if (ctx.params.size() > 1) {
        std::cout << "More args than <token> for " + ctx.command + ": " + ctx.command + " <token>"
                  << std::endl;
        return true;
    }
    return false;
}

// already : will be not in param => take it as a tail
// bool PingPongManager::onlyColon(const CommandProcessor::CommandContext &ctx)
// {
//     if (ctx.params[0] == ":") {
//         sendToClient(ctx.clientFd, ERR_NOORIGIN(ctx.nickname));
//         std::cout << "No specific : No origin" << std::endl;
//         return true;
//     }
//     return false;
// }

bool PingPongManager::validateCommand(const CommandProcessor::CommandContext &ctx,
                                      const std::string &expected)
{
    if (ctx.command != expected) {
        std::cout << "Expected " << expected << " but got " << ctx.command << std::endl;
        return false;
    }
    return true;
}

bool PingPongManager::validateParams(const CommandProcessor::CommandContext &ctx)
{
    if (EmptyToken(ctx))
        return (false);
    if (MoreThanOneToken(ctx))
        return (false);
    return (true);
}
void PingPongManager::sendPongResponse(const CommandProcessor::CommandContext &ctx)
{
    std::string response = ":" + SERVER_NAME + " PONG " + SERVER_NAME + " :" + ctx.params[0];
    sendToClient(ctx.clientFd, response);
}

void PingPongManager::ping(const CommandProcessor::CommandContext &ctx)
{
    if (!validateCommand(ctx, "PING"))
        return;
    if (rejectPingBeforeRegistered(ctx))
        return;
    if (!validateParams(ctx))
        return;
    sendPongResponse(ctx);
}

void PingPongManager::pong(const CommandProcessor::CommandContext &ctx)
{
    if (!validateCommand(ctx, "PONG"))
        return;
    if (!validateParams(ctx))
        return;

    handlePongFromClient(ctx.params[0]);
}

void PingPongManager::addPingToken(const std::string &token)
{
    _pingTokens[token] = std::chrono::steady_clock::now();
}

void PingPongManager::handlePongFromClient(const std::string &token)
{
    auto it = _pingTokens.find(token);
    if (it != _pingTokens.end()) {
        // auto now = std::chrono::steady_clock::now();
        // auto pingTime = it->second;
        // auto duration =
        // std::chrono::duration_cast<std::chrono::milliseconds>(now - pingTime).count();
        // std::cout << "PONG received from " << _nickname << " in " << duration << "ms" <<
        // std::endl;
        _pingTokens.erase(it);
    }
}

bool PingPongManager::checkPingTimeouts(int timeoutMs)
{
    auto now = std::chrono::steady_clock::now();

    for (auto it = _pingTokens.begin(); it != _pingTokens.end();) {
        auto duration =
            std::chrono::duration_cast<std::chrono::milliseconds>(now - it->second).count();
        if (duration > timeoutMs) {
            return true;
        }
        ++it;
    }
    return false;
}

void PingPongManager::sendPingToClient(Client &client)
{
    std::string token = "PING_" + std::to_string(time(NULL));
    addPingToken(token);

    sendToClient(client.getFd(), "PING " + token);
    std::cout << "Sending PING to " << client.getNickname() << ": " << token << std::endl;
}

void PingPongManager::sendPingToAllClients(ClientIndex &clients)
{
    clients.forEachClient([this](Client &client) { sendPingToClient(client); });
}

// check pong responses within 60s
void PingPongManager::checkAllPingTimeouts(int timeoutMs, ClientIndex &clients,
                                           ConnectionManager &connManager)
{
    // cannot reference in vector before initializing and also because it is modified while
    // iterating pointer is better for memory usage

    std::vector<Client *> clientsToDisconnect;
    clients.forEachClient([this, &clientsToDisconnect, timeoutMs](Client &client) {
        if (checkPingTimeouts(timeoutMs)) {
            std::cout << "Client " << client.getNickname() << " has no PONG response after "
                      << timeoutMs / 1000 << " seconds" << std::endl;
            clientsToDisconnect.push_back(&client);
        }
    });
    for (Client *client : clientsToDisconnect) {
        std::cout << "Client " << client->getNickname() << " has no PONG response after "
                  << timeoutMs << " seconds" << std::endl;
        connManager.markClientForDisconnection(client);
    }
}
