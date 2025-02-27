#include <channel.hpp>
#include <server.hpp>
#include <Client.hpp>

Channel::Channel()
    : name("Default")
    , modes("")
    , key("")
    , userLimit(0)
{
}

Channel::Channel(const std::string &name, Client *creator)
    : name(name)
    , modes("")
    , key("")
    , userLimit(0)
{
    // validate channel name
    join(creator);
    giveOp(creator);
}

void Channel::join(Client *client)
{
    if (!client)
        return;
    int fd = client->getFd();
    if (connectedClients.find(fd) == connectedClients.end()) {
        connectedClients[fd] = client;
    }
}

void Channel::leave(Client *client)
{
    if (!client)
        return;
    connectedClients.erase(client->getFd());
    ops.erase(client->getFd());
}

void Channel::toggleMode(char mode)
{
    size_t index = modes.find(mode);
    if (index == std::string::npos) {
        modes.push_back(mode);
    }
    else {
        modes.erase(index);
    }
}

void Channel::giveOp(Client *client)
{
    if (!client)
        return;
    ops[client->getFd()] = client;
}

bool Channel::hasMode(const char mode) const
{
    return modes.find(mode) != std::string::npos;
}
