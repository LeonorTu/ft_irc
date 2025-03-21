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
    , _createdTime(std::to_string(time(0)))
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
    std::string joinMessage = JOIN(client.getUserHost(), _channelName);

    _connectedClients.insert_or_assign(client.getNickname(), &client);
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
    std::string partMessage = PART(client.getUserHost(), _channelName, reason);

    std::string nick = client.getNickname();
    broadcastMessage(partMessage);
    _connectedClients.erase(nick);
    removeOp(nick);
    client.untrackChannel(this);
}

void Channel::quit(Client &client, const std::string &reason)
{
    if (!isOnChannel(client))
        return;
    std::string quitMessage = QUIT(client.getUserHost(), reason);

    std::string nick = client.getNickname();
    _connectedClients.erase(nick);
    removeOp(nick);
    broadcastMessage(quitMessage);
}

void Channel::invite(Client &inviter, Client &target)
{
    int inviterFd = inviter.getFd();
    std::string inviterName = inviter.getNickname();
    std::string targetName = target.getNickname();

    if (!isOnChannel(inviter)) {
        sendToClient(inviterFd, ERR_NOTONCHANNEL(inviterName, _channelName));
        return;
    }
    if (isOnChannel(target)) {
        sendToClient(inviterFd, ERR_USERONCHANNEL(inviterName, targetName, _channelName));
        return;
    }
    if (hasMode(ChannelMode::INVITE_ONLY) && !hasOp(inviter)) {
        sendToClient(inviterFd, ERR_CHANOPRIVSNEEDED(inviterName, _channelName));
        return;
    }

    _invites.insert_or_assign(targetName, &target);
    sendToClient(target.getFd(), INVITE(inviter.getUserHost(), targetName, _channelName));
    sendToClient(inviterFd, RPL_INVITING(inviterName, targetName, _channelName));
}

void Channel::kick(Client &kicker, Client &target, std::string const &reason)
{
    int kickerFd = kicker.getFd();
    std::string kickerName = kicker.getNickname();
    std::string targetName = target.getNickname();

    if (!isOnChannel(kicker)) {
        sendToClient(kickerFd, ERR_NOTONCHANNEL(kickerName, _channelName));
        return;
    }
    if (!isOnChannel(target)) {
        sendToClient(kickerFd, ERR_USERNOTINCHANNEL(kickerName, targetName, _channelName));
        return;
    }
    if (hasMode(ChannelMode::INVITE_ONLY) && !hasOp(kicker)) {
        sendToClient(kickerFd, ERR_CHANOPRIVSNEEDED(kickerName, _channelName));
        return;
    }
    std::string kickMessage = KICK(kicker.getUserHost(), targetName, _channelName, reason);
    broadcastMessage(kickMessage);
    _connectedClients.erase(targetName);
    removeOp(targetName);
    target.untrackChannel(this);
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
    broadcastMessage(TOPIC(client.getUserHost(), _channelName, _topic));
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

const std::string &Channel::getCreatedTime()
{
    return _createdTime;
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
    std::string modeMsg = MODE(client.getUserHost(), _channelName, modeStr, param);

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
        case ChannelMode::INVITE_ONLY:
            _invites.clear();
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

void Channel::printModes(Client &client)
{
    std::string modeString;
    if (!_modes.empty())
        modeString = '+' + _modes;
    for (char mode : _modes) {
        if (mode == 'k')
            modeString = modeString + ' ' + _key;
        else if (mode == 'l')
            modeString = modeString + ' ' + std::to_string(_userLimit);
    }
    sendToClient(client.getFd(), RPL_CHANNELMODEIS(client.getNickname(), _channelName, modeString));
    sendToClient(client.getFd(),
                 RPL_CREATIONTIME(client.getNickname(), _channelName, getCreatedTime()));
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
        // std::cout << "Sending message to " << client->getNickname() << ": " << message << std::endl;
    }
}

void Channel::enableMode(ChannelMode mode)
{
    if (mode == ChannelMode::OP)
        return;
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

void Channel::removeOp(const std::string &nick, const std::string &modeMsg)
{
    if (_ops.find(nick) != _ops.end()) {
        _ops.erase(nick);
        broadcastMessage(modeMsg);
    }
}
