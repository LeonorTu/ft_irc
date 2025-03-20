#include <PongManager.hpp>

PongManager::PongManager() {};
PongManager::~PongManager() {};

void PongManager::addPingToken(const std::string &token)
{
    _pingTokens[token] = std::chrono::steady_clock::now();
}

void PongManager::handlePongFromClient(const std::string &token)
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

bool PongManager::checkPingTimeouts(int timeoutMs)
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

void PongManager::sendPingToClient(Client &client)
{
    std::string token = "PING_" + std::to_string(time(NULL));
    addPingToken(token);

    sendToClient(client.getFd(), "PING " + token);
    std::cout << "Sending PING to " << client.getNickname() << ": " << token << std::endl;
}

void PongManager::sendPingToAllClients(ClientIndex &clients)
{
    clients.forEachClient([this](Client &client) { sendPingToClient(client); });
}

// check pong responses within 60s
void PongManager::checkAllPingTimeouts(int timeoutMs, ClientIndex &clients,
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

// std::unordered_map<std::string, std::chrono::steady_clock::time_point>
// & PongManager::getPingTokens(){
//     return _pingTokens;
// }