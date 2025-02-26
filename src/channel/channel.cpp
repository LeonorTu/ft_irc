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

Channel::Channel(const std::string &name, Client &creator)
    : name(name)
    , modes("")
    , key("")
    , userLimit(0)
{
    // validate channel name
    join(creator);
    giveOp(creator);
}

void Channel::join(Client &client)
{
    int fd = client.getFd();
    if (connectedClients.find(fd) == connectedClients.end()) {
        connectedClients.at(fd) = client;
    }
}

void Channel::leave(Client &client)
{
    connectedClients.erase(client.getFd());
    ops.erase(client.getFd());
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

void Channel::giveOp(Client &creator)
{
    ops.at(creator.getFd()) = creator;
}

bool Channel::hasMode(const char mode) const
{
    return modes.find(mode) != std::string::npos;
}
