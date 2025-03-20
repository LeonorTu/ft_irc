#include <Channel.hpp>
#include <Client.hpp>
#include <responses.hpp>

Channel::Channel(const std::string &name, Client &creator)
    : _channelName(name)
    , _topic("")
    , _topicAuthor("")
    , _topicTime("")
    , _modes("")
    , _key("")
    , _userLimit(0)
{
    _ops.insert_or_assign(creator.getNickname(), &creator);
    setMode(creator, true, ChannelMode::OP, creator.getNickname());
    join(creator);
}

    Channel::~Channel()
{
    for (auto &[_, client] : _connectedClients) {
        client->untrackChannel(this);
    }
}

void Channel::join(Client &client, const std::string &key)
{
    if (!isJoinable(client, key))
        return;
    std::string nick = client.getNickname();
    std::string joinMessage = JOIN(nick, _channelName);

    _connectedClients.insert_or_assign(nick, &client);
    client.trackChannel(this);
    removeFromInvites(client);

    // required server reply on join success
    broadcastMessage(joinMessage);
    sendTopic(client);
    sendNameReply(client);
}

void Channel::part(Client &client, const std::string &reason)
{
    if (!isOnChannel(client)) {
        sendToClient(client.getFd(), ERR_NOTONCHANNEL(client.getNickname(), _channelName));
        return;
    }
    std::string nick = client.getNickname();
    std::string partMessage = PART(nick, _channelName, reason);

    broadcastMessage(partMessage);
    _connectedClients.erase(nick);
    removeOp(nick);
    client.untrackChannel(this);
}

void Channel::quit(Client &client, const std::string &reason)
{
    if (!isOnChannel(client))
        return;
    std::string nick = client.getNickname();
    std::string quitMessage = QUIT(client.getNickname(), reason);

    _connectedClients.erase(nick);
    removeOp(nick);
    client.untrackChannel(this);
    broadcastMessage(quitMessage);
}

void Channel::invite(Client &inviter, Client &target)
{
    if (!isOnChannel(inviter))
        sendToClient(inviter.getFd(), ERR_NOTONCHANNEL(inviter.getNickname(), _channelName));
    if (isOnChannel(target))
        sendToClient(inviter.getFd(),
                     ERR_USERONCHANNEL(inviter.getNickname(), target.getNickname(), _channelName));
    if (hasMode(ChannelMode::INVITE_ONLY) && !hasOp(inviter))
        sendToClient(inviter.getFd(), ERR_CHANOPRIVSNEEDED(inviter.getNickname(), _channelName));

    auto it = _invites.find(target.getNickname());
    if (it != _invites.end()) {
        _invites.insert_or_assign(target.getNickname(), it->second);
    }
    sendToClient(target.getFd(), INVITE(inviter.getNickname(), target.getNickname(), _channelName));
    sendToClient(inviter.getFd(),
                 RPL_INVITING(inviter.getNickname(), target.getNickname(), _channelName));
}

void Channel::changeTopic(Client &client, std::string &newTopic)
{
    if (!isOnChannel(client)) {
        sendToClient(client.getFd(), ERR_NOTONCHANNEL(client.getNickname(), _channelName));
        return;
    }
    if (hasMode(ChannelMode::PROTECTED_TOPIC) && !hasOp(client)) {
        sendToClient(client.getFd(), ERR_CHANOPRIVSNEEDED(client.getNickname(), _channelName));
        return;
    }
    _topic = newTopic;
    _topicAuthor = client.getNickname();
    _topicTime = std::to_string(time(0));
    for (auto &[_, client] : _connectedClients) {
        sendTopic(*client);
    }
}

void Channel::checkTopic(Client &client)
{
    if (!isOnChannel(client)) {
        sendToClient(client.getFd(), ERR_NOTONCHANNEL(client.getNickname(), _channelName));
        return;
    }
    sendTopic(client);
}

const std::string &Channel::getName() const
{
    return _channelName;
}

bool Channel::hasMode(ChannelMode mode) const
{
    return _modes.find(static_cast<char>(mode)) != std::string::npos;
}

bool Channel::hasMode(const char mode) const
{
    return hasMode(static_cast<ChannelMode>(mode));
}

void Channel::setMode(Client &client, bool enable, ChannelMode mode, std::string param)
{
    if (!hasOp(client)) {
        sendToClient(client.getFd(), ERR_CHANOPRIVSNEEDED(client.getNickname(), _channelName));
        return;
    }
    // For basic modes (i, t), keep the check
    if (mode == ChannelMode::INVITE_ONLY || mode == ChannelMode::PROTECTED_TOPIC) {
        if (hasMode(mode) == enable)
            return;
    }
    // For parametrized modes with same values, skip
    if (mode == ChannelMode::KEY && enable && hasMode(mode) && this->_key == param)
        return;
    if (mode == ChannelMode::LIMIT && enable && hasMode(mode) &&
        this->_userLimit == std::stoul(param))
        return;

    std::string operation = enable ? "+" : "-";
    std::string modeStr = operation + static_cast<char>(mode);
    std::string modeMsg = MODE(client.getNickname(), _channelName, modeStr, param);

    if (enable) {
        enableMode(mode);
        // Handle mode-specific parameters
        switch (mode) {
        case ChannelMode::KEY:
            this->_key = param;
            break;
        case ChannelMode::LIMIT:
            if (!param.empty())
                this->_userLimit = std::stoi(param);
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

void Channel::setMode(Client &client, bool enable, const char mode, std::string param)
{
    ChannelMode channelMode = static_cast<ChannelMode>(mode);
    setMode(client, enable, channelMode, param);
}

bool Channel::isEmpty() const
{
    return _connectedClients.empty();
}

void Channel::broadcastMessage(const std::string &message)
{
    if (message.empty())
        return;
    for (auto &[_, client] : _connectedClients) {
        sendToClient(client->getFd(), message);
    }
}

void Channel::enableMode(ChannelMode mode)
{
    if (_modes.find(static_cast<char>(mode)) == std::string::npos) {
        _modes.push_back(mode);
    }
}

void Channel::disableMode(ChannelMode mode)
{
    size_t index = _modes.find(static_cast<char>(mode));
    if (index == std::string::npos) {
        return;
    }
    _modes.erase(index, 1);
}

std::string Channel::prefixNick(Client &client)
{
    std::string nick = client.getNickname();
    if (hasOp(client))
        return "@" + nick;
    return nick;
}

void Channel::sendNameReply(Client &client)
{
    std::string nameReply;
    std::string nextNick;

    for (auto &[_, memberClient] : _connectedClients) {
        nextNick = prefixNick(*memberClient);
        // + 3 to account for <space>\r\n
        if (nameReply.size() + nextNick.size() + 3 > MSG_BUFFER_SIZE) {
            sendToClient(client.getFd(), nameReply);
            nameReply.erase();
        }
        if (nameReply.empty()) {
            nameReply = RPL_NAMREPLY(client.getNickname(), _channelName, nextNick);
            continue;
        }
        nameReply.append(" " + nextNick);
    }
    sendToClient(client.getFd(), nameReply);
    sendToClient(client.getFd(), RPL_ENDOFNAMES(client.getNickname(), _channelName));
}

void Channel::sendTopic(Client &client)
{
    int fd = client.getFd();

    if (_topic.empty()) {
        sendToClient(fd, RPL_NOTOPIC(client.getNickname(), _channelName));
    }
    else {
        sendToClient(fd, RPL_TOPIC(client.getNickname(), _channelName, _topic));
        sendToClient(
            fd, RPL_TOPICWHOTIME(client.getNickname(), _channelName, _topicAuthor, _topicTime));
    }
}

bool Channel::hasOp(Client &client)
{
    return _ops.find(client.getNickname()) != _ops.end();
}

bool Channel::isInvited(Client &client)
{
    return _invites.find(client.getNickname()) != _invites.end();
}

bool Channel::isJoinable(Client &client, std::string key)
{
    int fd = client.getFd();

    if (hasMode(ChannelMode::INVITE_ONLY) && !isInvited(client)) {
        sendToClient(fd, ERR_INVITEONLYCHAN(client.getNickname(), _channelName));
        return false;
    }
    if (hasMode(ChannelMode::KEY) && key != _key) {
        sendToClient(fd, ERR_BADCHANNELKEY(client.getNickname(), _channelName));
        return false;
    }
    if (hasMode(ChannelMode::LIMIT) && _connectedClients.size() >= _userLimit) {
        sendToClient(fd, ERR_CHANNELISFULL(client.getNickname(), _channelName));
        return false;
    }
    if (isOnChannel(client)) {
        return false;
    }
    return true;
}

bool Channel::isOnChannel(Client &client)
{
    return _connectedClients.find(client.getNickname()) != _connectedClients.end();
}

void Channel::removeFromInvites(Client &client)
{
    if (_invites.find(client.getNickname()) != _invites.end())
        _invites.erase(client.getNickname());
}

void Channel::addOp(std::string &nick, const std::string &modeMsg)
{
    auto it = _connectedClients.find(nick);
    if (it != _connectedClients.end() && !hasOp(*it->second)) {
        _ops.insert_or_assign(nick, it->second);
        broadcastMessage(modeMsg);
    }
}

void Channel::removeOp(std::string &nick, const std::string &modeMsg)
{
    if (_ops.find(nick) != _ops.end()) {
        _ops.erase(nick);
        broadcastMessage(modeMsg);
    }
}
