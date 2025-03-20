#include <ConnectionManager.hpp>
#include <PongManager.hpp>


PongManager::PongManager() {};
PongManager::~PongManager() {};

void PongManager::handlePongFromClient(const std::string &token, Client &client)
{
    if (client.getLastPingToken() == token) {
        client.noPongWait();
        client.updateActivityTime();
    }
}

bool PongManager::checkPingTimeouts(int timeoutMs, Client &client)
{
    if (!client.isWaitingForPong())
        return false;
    return client.getTimeSinceLastPing() > timeoutMs;
}

void PongManager::sendPingToClient(Client &client)
{
    std::string token = "PING_" + std::to_string(client.getFd()) + "_" + std::to_string(time(NULL));
    client.markPingSent(token);
    sendToClient(client.getFd(), "PING " + token);
    std::cout << "Sending PING to " << client.getNickname() << ": " << token << std::endl;
}

void PongManager::sendPingToAllClients(ClientIndex &clients)
{
    clients.forEachClient([this](Client &client) {
        if (client.isWaitingForPong())
            return;
        sendPingToClient(client);
    });
}

// check pong responses within 60s
void PongManager::checkAllPingTimeouts(int timeoutMs, ClientIndex &clients,
                                       ConnectionManager &connManager)
{
    // cannot reference in vector before initializing and also because it is modified while
    // iterating pointer is better for memory usage
    std::vector<Client *> clientsToDisconnect;
    clients.forEachClient([this, &clientsToDisconnect, timeoutMs](Client &client) {
        if (checkPingTimeouts(timeoutMs, client)) {
            std::cout << "Client " << client.getNickname() << " has no PONG response after "
                      << timeoutMs / 1000 << " seconds" << std::endl;
            clientsToDisconnect.push_back(&client);
        }
    });
    for (Client *client : clientsToDisconnect) {
        connManager.disconnectClient(*client, "Ping timeout: " + std::to_string(timeoutMs / 1000) +
                                                  " seconds");
    }
}
