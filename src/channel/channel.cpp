#include <channel.hpp>
#include <server.hpp>
#include <Client.hpp>
#include <responses.hpp>
#include <common.hpp>

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

Channel::~Channel()
{
    for (auto &[fd, client] : connectedClients) {
        client->untrackChannel(this);
    }
}

void Channel::join(Client *client, std::string key)
{
    if (!client)
        return;
    int fd = client->getFd();
    if (key != this->key) {
        sendToClient(fd, ERR_BADCHANNELKEY(client->getNickname(), name));
        return;
    }

    if (connectedClients.find(fd) == connectedClients.end()) {
        connectedClients.at(fd) = client;
    }
    client->trackChannel(this);
    
    sendToClient(fd, RPL_TOPIC(client->getNickname(), this->name, this->topic));
    nameReply(client);
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

void Channel::nameReply(Client *client)
{

const std::string &Channel::getName() const
{
    return this->name;
}

bool Channel::hasMode(const char mode) const
{
    return modes.find(mode) != std::string::npos;
}
