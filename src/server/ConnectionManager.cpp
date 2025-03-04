#include <ConnectionManager.hpp>
#include <common.hpp>

ConnectionManager::ConnectionManager(ClientIndex &clients, SocketManager &socketManager, EventLoop &EventLoop)
    : _clients(clients)
    , _socketManager(socketManager)
    , _EventLoop(EventLoop)
{}

void ConnectionManager::handleNewClient()
{
    sockaddr_in clientAddr;

    int clientFd = _socketManager.acceptConnection(&clientAddr);
    std::string ip = inet_ntoa(clientAddr.sin_addr);
    //add new client into ClientIndex
    _clients.add(clientFd);
    Client &client = _clients.getByFd(clientFd);
    client.setIp(ip);
    //add new client to epoll list
    _EventLoop.addToWatch(clientFd, EPOLLIN | EPOLLET);
    std::cout << "New client" << std::endl;
    std::cout << "  Socket: " << clientFd << std::endl;
    std::cout << "  IP:     " << ip << std::endl;
}

void ConnectionManager::disconnectClient(Client &client)
{
    _socketManager.closeConnection(client.getFd());
    _EventLoop.removeFromWatch(client.getFd());
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

        messageBuf.append(buffer, bytesRead);
    }
    extractFullMessages(client, messageBuf);
}

void ConnectionManager::extractFullMessages(Client &client, std::string &messageBuffer)
{
    size_t pos;
    while ((pos = messageBuffer.find("\r\n")) != std::string::npos) {
        // completed message is already without \r\n
        std::string completedMessage = messageBuffer.substr(0, pos);
        messageBuffer.erase(0, pos + 2);
        // call commandhandler, executer or whatever
        std::cout << "Recieved Message from " << client.getFd() << " : " << completedMessage << std::endl;
    }
    handleOversized(client, messageBuffer);
}

void ConnectionManager::handleOversized(Client &client, std::string &messageBuffer)
{
    if (messageBuffer.size() > MSG_BUFFER_SIZE) {
        std::cerr << "Message too large from client: " << client.getFd() << std::endl;
        std::cerr << "Truncating message" << std::endl;
        std::string truncatedMessage = messageBuffer.substr(0, MSG_BUFFER_SIZE - 2);
        messageBuffer.clear();
        // call commandhandler, executer or whatever
        std::cout << "Recieved Message from " << client.getFd() << " : " << truncatedMessage << std::endl;
    }
}
