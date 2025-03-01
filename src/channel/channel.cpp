#include <channel.hpp>
#include <server.hpp>
#include <Client.hpp>
#include <responses.hpp>
#include <common.hpp>

Channel::Channel(const std::string &name, Client *creator)
    : channelName(name)
    , modes("")
    , key("")
    , userLimit(0)
    , topic("")
{
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
    if (!isJoinable(client, key))
        return;
    int fd = client->getFd();
    JOIN(client->getNickname(), channelName);
    connectedClients.at(fd) = client;
    client->trackChannel(this);
    sendTopic(client);
    sendNameReply(client);
}

void Channel::leave(Client *client)
{
    if (!client)
        return;
    connectedClients.erase(client->getFd());
    ops.erase(client->getFd());
}

void Channel::changeTopic(Client *client, std::string &newTopic)
{
    if (!isOnChannel(client)) {
        ERR_NOTONCHANNEL(client->getNickname(), channelName);
        return;
    }
    if (hasMode('t') && !hasOp(client)) {
        ERR_CHANOPRIVSNEEDED(client->getNickname(), channelName);
        return;
    }
    topic = newTopic;
    topicAuthor = client->getNickname();
    topicTime = std::to_string(time(0));
    for (auto &[fd, client] : connectedClients) {
        sendTopic(client);
    }
}

void Channel::giveOp(Client *client)
{
    if (!client)
        return;
    ops.at(client->getFd()) = client;
}

void Channel::changeKey(Client *client, std::string key)
{
    if (!hasOp(client)) {
        ERR_CHANOPRIVSNEEDED(client->getNickname(), channelName);
        return;
    }
    this->key = key;
}

const std::string &Channel::getName() const
{
    return channelName;
}

bool Channel::hasMode(const char mode) const
{
    return modes.find(mode) != std::string::npos;
}

void Channel::enableMode(char mode)
{
    if (modes.find(mode) == std::string::npos) {
        modes.push_back(mode);
    }
}

void Channel::disableMode(char mode)
{
    size_t index = modes.find(mode);
    if (index == std::string::npos) {
        return;
    }
    modes.erase(index);
}

std::string Channel::prefixNick(Client *client)
{
    std::string nick = client->getNickname();
    if (hasOp(client))
        return "@" + nick;
    return nick;
}

void Channel::sendNameReply(Client *client)
{
    std::string nameReply;
    std::string nextNick;

    for (auto &[fd, memberClient] : connectedClients) {
        nextNick = prefixNick(client);
        // + 3 to account for <space>\r\n
        if (nameReply.size() + nextNick.size() + 3 > MSG_BUFFER_SIZE) {
            sendToClient(client->getFd(), nameReply);
            nameReply.erase();
        }
        if (nameReply.empty()) {
            nameReply = RPL_NAMREPLY(client->getNickname(), channelName, nextNick);
            continue;
        }
        nameReply.append(" " + nextNick);
    }
    sendToClient(client->getFd(), RPL_ENDOFNAMES(client->getNickname(), channelName));
}

void Channel::sendTopic(Client *client)
{
    int fd = client->getFd();

    if (topic.empty()) {
        sendToClient(fd, RPL_NOTOPIC(client->getNickname(), channelName));
    }
    else {
        sendToClient(fd, RPL_TOPIC(client->getNickname(), channelName, topic));
        sendToClient(fd, RPL_TOPICWHOTIME(client->getNickname(), channelName, topicAuthor, topicTime));
    }
}

bool Channel::hasOp(Client *client)
{
    return ops.find(client->getFd()) != ops.end();
}

bool Channel::isInvited(Client *client)
{
    return invites.find(client->getNickname()) != invites.end();
}

bool Channel::isJoinable(Client *client, std::string key)
{
    int fd = client->getFd();

    if (hasMode('k') && key != this->key) {
        sendToClient(fd, ERR_BADCHANNELKEY(client->getNickname(), channelName));
        return false;
    }
    if (hasMode('i') && !isInvited(client)) {
        sendToClient(fd, ERR_INVITEONLYCHAN(client->getNickname(), channelName));
        return false;
    }
    if (hasMode('l') && connectedClients.size() > userLimit)
        sendToClient(fd, ERR_CHANNELISFULL(client->getNickname(), channelName));
    if (connectedClients.find(fd) != connectedClients.end()) {
        return false;
    }
    return true;
}

bool Channel::isOnChannel(Client *client)
{
    return connectedClients.find(client->getFd()) != connectedClients.end();
}
