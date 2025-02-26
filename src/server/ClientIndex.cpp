#include <ClientIndex.hpp>
#include <Client.hpp>

ClientIndex::~ClientIndex()
{
    byFd.clear();
    byNick.clear();
    std::cout << "clientIndex cleared" << std::endl;
}

void ClientIndex::add(Client *client)
{
    if (!client)
        return;

    byFd[client->getFd()] = client;
    byNick[client->getNickname()] = client;
}

void ClientIndex::remove(Client *client)
{
    if (!client)
        return;

    byFd.erase(client->getFd());
    byNick.erase(client->getNickname());
}

void ClientIndex::updateNick(const std::string &oldNick, const std::string &newNick)
{
    auto it = byNick.find(oldNick);
    if (it == byNick.end())
        return;

    Client *client = it->second;
    byNick.erase(oldNick);
    byNick[newNick] = client;
}

void ClientIndex::addUnregistered(Client *client)
{
    if (!client)
        return;
    byFd[client->getFd()] = client;
}

Client *ClientIndex::getByFd(int fd) const
{
    auto it = byFd.find(fd);
    return (it != byFd.end()) ? it->second : nullptr;
}

Client *ClientIndex::getByNick(const std::string &nick) const
{
    auto it = byNick.find(nick);
    return (it != byNick.end()) ? it->second : nullptr;
}

std::unordered_map<int, Client *> &ClientIndex::getClientsForCleanup()
{
    return byFd;
}

bool ClientIndex::nickExists(const std::string &nick) const
{
    return byNick.find(nick) != byNick.end();
}

size_t ClientIndex::size() const
{
    return byFd.size();
}
