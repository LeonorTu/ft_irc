#include <channel.hpp>
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
    ops.insert_or_assign(creator->getNickname(), creator);
    setMode(creator, true, ChannelMode::OP, creator->getNickname());
    join(creator);
}

Channel::~Channel()
{
    for (auto &[_, client] : connectedClients) {
        client->untrackChannel(this);
    }
}

void Channel::join(Client *client, const std::string &key)
{
    if (!client)
        return;
    if (!isJoinable(client, key))
        return;
    std::string nick = client->getNickname();
    std::string joinMessage = JOIN(nick, channelName);

    connectedClients.insert_or_assign(nick, client);
    client->trackChannel(this);
    removeFromInvites(client);

    // required server reply on join success
    broadcastMessage(joinMessage);
    sendTopic(client);
    sendNameReply(client);
}

void Channel::part(Client *client, const std::string &reason)
{
    if (!client)
        return;
    if (!isOnChannel(client)) {
        ERR_NOTONCHANNEL(client->getNickname(), channelName);
        return;
    }
    std::string nick = client->getNickname();
    std::string partMessage = PART(nick, channelName, reason);

    broadcastMessage(partMessage);
    connectedClients.erase(nick);
    removeOp(nick);
    client->untrackChannel(this);
}

void Channel::quit(Client *client, const std::string &reason)
{
    if (!client)
        return;
    if (!isOnChannel(client))
        return;
    std::string nick = client->getNickname();
    std::string quitMessage = QUIT(client->getNickname(), reason);

    connectedClients.erase(nick);
    removeOp(nick);
    client->untrackChannel(this);
    broadcastMessage(quitMessage);
}

void Channel::changeTopic(Client *client, std::string &newTopic)
{
    if (!isOnChannel(client)) {
        ERR_NOTONCHANNEL(client->getNickname(), channelName);
        return;
    }
    if (hasMode(ChannelMode::PROTECTED_TOPIC) && !hasOp(client)) {
        ERR_CHANOPRIVSNEEDED(client->getNickname(), channelName);
        return;
    }
    topic = newTopic;
    topicAuthor = client->getNickname();
    topicTime = std::to_string(time(0));
    for (auto &[_, client] : connectedClients) {
        sendTopic(client);
    }
}

void Channel::checkTopic(Client *client)
{
    if (!isOnChannel(client)) {
        ERR_NOTONCHANNEL(client->getNickname(), channelName);
        return;
    }
    sendTopic(client);
}

const std::string &Channel::getName() const
{
    return channelName;
}

bool Channel::hasMode(ChannelMode mode) const
{
    return modes.find(static_cast<char>(mode)) != std::string::npos;
}

bool Channel::hasMode(const char mode) const
{
    return hasMode(static_cast<ChannelMode>(mode));
}

void Channel::setMode(Client *client, bool enable, ChannelMode mode, std::string param)
{
    if (!hasOp(client)) {
        ERR_CHANOPRIVSNEEDED(client->getNickname(), channelName);
        return;
    }
    // For basic modes (i, t), keep the check
    if (mode == ChannelMode::INVITE_ONLY || mode == ChannelMode::PROTECTED_TOPIC) {
        if (hasMode(mode) == enable)
            return;
    }
    // For parametrized modes with same values, skip
    if (mode == ChannelMode::KEY && enable && hasMode(mode) && this->key == param)
        return;
    if (mode == ChannelMode::LIMIT && enable && hasMode(mode) && this->userLimit == std::stoi(param))
        return;

    std::string operation = enable ? "+" : "-";
    std::string modeStr = operation + static_cast<char>(mode);
    std::string modeMsg = MODE(client->getNickname(), channelName, modeStr, param);

    if (enable) {
        enableMode(mode);
        // Handle mode-specific parameters
        switch (mode) {
        case ChannelMode::KEY:
            this->key = param;
            break;
        case ChannelMode::LIMIT:
            if (!param.empty())
                this->userLimit = std::stoi(param);
            break;
        case ChannelMode::OP:
            addOp(param, modeMsg);
            return;
        default:
            break;
        }
    }
    else {
        disableMode(mode);
        switch (mode) {
        case ChannelMode::OP:
            removeOp(param, modeMsg);
            return;
        default:
            break;
        }
    }
    broadcastMessage(modeMsg);
}

bool Channel::isEmpty() const
{
    return connectedClients.empty();
}

void Channel::broadcastMessage(const std::string &message)
{
    if (message.empty())
        return;
    for (auto &[_, client] : connectedClients) {
        sendToClient(client->getFd(), message);
    }
}

void Channel::enableMode(ChannelMode mode)
{
    if (modes.find(static_cast<char>(mode)) == std::string::npos) {
        modes.push_back(mode);
    }
}

void Channel::disableMode(ChannelMode mode)
{
    size_t index = modes.find(static_cast<char>(mode));
    if (index == std::string::npos) {
        return;
    }
    modes.erase(index, 1);
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

    for (auto &[_, memberClient] : connectedClients) {
        nextNick = prefixNick(memberClient);
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
    sendToClient(client->getFd(), nameReply);
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
    return ops.find(client->getNickname()) != ops.end();
}

bool Channel::isInvited(Client *client)
{
    return invites.find(client->getNickname()) != invites.end();
}

bool Channel::isJoinable(Client *client, std::string key)
{
    int fd = client->getFd();

    if (hasMode(ChannelMode::INVITE_ONLY) && !isInvited(client)) {
        sendToClient(fd, ERR_INVITEONLYCHAN(client->getNickname(), channelName));
        return false;
    }
    if (hasMode(ChannelMode::KEY) && key != this->key) {
        sendToClient(fd, ERR_BADCHANNELKEY(client->getNickname(), channelName));
        return false;
    }
    if (hasMode(ChannelMode::LIMIT) && connectedClients.size() >= userLimit) {
        sendToClient(fd, ERR_CHANNELISFULL(client->getNickname(), channelName));
        return false;
    }
    if (isOnChannel(client)) {
        return false;
    }
    return true;
}

bool Channel::isOnChannel(Client *client)
{
    return connectedClients.find(client->getNickname()) != connectedClients.end();
}

void Channel::removeFromInvites(Client *client)
{
    if (invites.find(client->getNickname()) != invites.end())
        invites.erase(client->getNickname());
}

void Channel::addOp(std::string &nick, const std::string &modeMsg)
{
    auto it = connectedClients.find(nick);
    if (it != connectedClients.end() && !hasOp(it->second)) {
        ops.insert_or_assign(nick, it->second);
        broadcastMessage(modeMsg);
    }
}

void Channel::removeOp(std::string &nick, const std::string &modeMsg)
{
    if (ops.find(nick) != ops.end()) {
        ops.erase(nick);
        broadcastMessage(modeMsg);
    }
}
