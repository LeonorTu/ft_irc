#include <ConnectionManager.hpp>
#include <common.hpp>
#include <responses.hpp>
#include <Channel.hpp>

ConnectionManager::ConnectionManager(SocketManager &socketManager, EventLoop &EventLoop,
                                     ClientIndex &clients)
    : _clients(clients)
    , _socketManager(socketManager)
    , _EventLoop(EventLoop)
    , _commandProcessor(CommandProcessor())
{}

ConnectionManager::~ConnectionManager()
{
    disconnectAllClients();
}

void ConnectionManager::handleNewClient()
{
    sockaddr_in clientAddr;

    int clientFd = _socketManager.acceptConnection(&clientAddr);
    std::string ip = inet_ntoa(clientAddr.sin_addr);
    // add new client into ClientIndex
    _clients.add(clientFd);
    Client &client = _clients.getByFd(clientFd);
    client.setIp(ip);
    // add new client to epoll list
    _EventLoop.addToWatch(clientFd);
    std::cout << "New client" << std::endl;
    std::cout << "  Socket: " << clientFd << std::endl;
    std::cout << "  IP:     " << ip << std::endl;
}

void ConnectionManager::disconnectClient(Client &client)
{
    _EventLoop.removeFromWatch(client.getFd());
    _socketManager.closeConnection(client.getFd());
    _clients.remove(client);
}

void ConnectionManager::recieveData(int clientFd)
{
    Client &client = _clients.getByFd(clientFd);

    char buffer[MSG_BUFFER_SIZE];
    std::string &messageBuf = client.getMessageBuf();
    while (true) {
        int bytesRead = recv(clientFd, buffer, sizeof(buffer), 0);

        if (bytesRead < 0) {
            if (errno == EPIPE) {
                std::cerr << "Broken pipe when sending to client " << clientFd << std::endl;
                return;
            }
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                // No more data, exit the loop
                break;
            }
            else {
                // Handle error
                disconnectClient(client);
                return;
            }
        }
        else if (bytesRead == 0) {
            // Client disconnected
            disconnectClient(client);
            return;
        }
        client.updateActivityTime();
        messageBuf.append(buffer, bytesRead);
    }
    extractFullMessages(client, messageBuf);
}

// extract a valid IRC message with /r/n ending, send the message as std::string to commandHandler
// command handler will get the message WIHTOUT /r/n
void ConnectionManager::extractFullMessages(Client &client, std::string &messageBuffer)
{
    if (messageBuffer.size() > MSG_BUFFER_SIZE) {
        handleOversized(client, messageBuffer);
        return;
    }
    size_t pos;
    while ((pos = messageBuffer.find("\n")) != std::string::npos) {
        size_t end = pos;
        if (end > 0 && messageBuffer[end - 1] == '\r') {
            end--;
        }
        std::string completedMessage = messageBuffer.substr(0, end);
        messageBuffer.erase(0, pos + 1);
        // Call the command handler, currently just printing out the log message.
        _commandProcessor.parseCommand(client, completedMessage);
        client.updateActivityTime();
    }
}

void ConnectionManager::disconnectAllClients()
{
    _clients.forEachClient([this](Client &client) { disconnectClient(client); });
}

// if the message is over buffer limit, truncate and delete therest of the message.
void ConnectionManager::handleOversized(Client &client, std::string &messageBuffer)
{
    if (messageBuffer.size() > MSG_BUFFER_SIZE) {
        std::cerr << "Message too large from client: " << client.getFd() << std::endl;
        std::cerr << "Truncating message" << std::endl;
        std::string truncatedMessage = messageBuffer.substr(0, MSG_BUFFER_SIZE - 2);
        messageBuffer.clear();
        // call commandhandler, executer or whatever
        _commandProcessor.parseCommand(client, truncatedMessage);
    }
}

std::vector<Client *> &ConnectionManager::getDisconnectedClients()
{
    return (_clientsToDisconnect);
}

void ConnectionManager::listClients(ClientIndex &clients)
{
    int count = 0;
    clients.forEachClient([&count](Client &client) {
        std::cout << "New client" << std::endl;
        std::cout << "  Nick name: " << client.getNickname() << std::endl;
        std::cout << "  User name: " << client.getUsername() << std::endl;
        std::cout << "  Real name: " << client.getRealname() << std::endl;
        std::cout << "  IsRegistered: " << (client.getIsRegistered() ? "Yes" : "No") << std::endl;
        count++;
    });
}

void ConnectionManager::rmDisconnectedClients()
{
    // std::cout << "\nBefore disconnecting clients: " << _clients.size() << std::endl << std::endl;
    // listClients();

    // will delete the client that timed out from the list
    for (Client *client : _clientsToDisconnect) {
        std::unordered_map<std::string, Channel *> channels = client->getMyChannels();
        for (auto &[_, channel] : channels) {
            std::string response = client->getNickname() + "no longer connected";
            channel->broadcastMessage(response);
            client->untrackChannel(channel);
        }
        std::cout << "Client " << client->getNickname() << " deleted" << std::endl;
        disconnectClient(*client);
    }
    // std::cout << "\nRemained clients: " << _clients.size() << std::endl << std::endl;
    // listClients();
    // std::cout << "End of listing" << std::endl;
}

void ConnectionManager::markClientForDisconnection(Client *client)
{
    // 중복 방지
    for (auto it = _clientsToDisconnect.begin(); it != _clientsToDisconnect.end(); ++it) {
        if (*it == client)
            return;
    }
    _clientsToDisconnect.push_back(client);
    std::cout << "Client " << client->getNickname() << " marked for disconnection" << std::endl;
}