#include <CommandRunner.hpp>
#include <unordered_set>
#include <array>

std::unordered_map<std::string, void (CommandRunner::*)()> CommandRunner::_commandRunners;

CommandRunner::CommandRunner(const MessageParser::CommandContext &ctx)
    : _server(Server::getInstance())
    , _clients(_server.getClients())
    , _channels(_server.getChannels())
    , _client(_clients.getByFd(ctx.clientFd))
    , _PongManager(_server.getPongManager())
    , _command(ctx.command)
    , _clientFd(ctx.clientFd)
    , _nickname(_client.getNickname())
    , _userHost(_client.getUserHost())
    , _messageSource(ctx.source)
    , _params(ctx.params)
    , _targets({})
    , _message("")
{}

bool CommandRunner::validateCommandAccess()
{
    static const std::unordered_set<std::string> duplicateRegistration = {"PASS", "USER"};
    static const std::unordered_set<std::string> alwaysAllowedCommands = {"PASS", "QUIT", "CAP"};
    static const std::unordered_set<std::string> preRegistrationCommands = {"NICK", "USER", "PONG"};

    if (duplicateRegistration.find(_command) != duplicateRegistration.end()) {
        if (_client.getIsRegistered()) {
            sendToClient(_clientFd, ERR_ALREADYREGISTERED(_nickname));
            return false;
        }
    }
    if (alwaysAllowedCommands.find(_command) != alwaysAllowedCommands.end()) {
        return true;
    }
    if (!_client.getPasswordVerified()) {
        sendToClient(_clientFd, ERR_NOTREGISTERED(_nickname));
        return false;
    }
    if (!_client.getIsRegistered() &&
        preRegistrationCommands.find(_command) != preRegistrationCommands.end()) {
        return true;
    }
    if (!_client.getIsRegistered()) {
        sendToClient(_clientFd, ERR_NOTREGISTERED(_nickname));
        return false;
    }
    return true;
}

void CommandRunner::execute()
{
    if (!validateCommandAccess()) {
        return;
    }

    auto commandIterator = _commandRunners.find(_command);
    if (commandIterator != _commandRunners.end()) {
        // Extract the command function pointer from the map
        auto commandFunction = commandIterator->second;
        (this->*commandFunction)();
    }
    else {
        sendToClient(_clientFd, ERR_UNKNOWNCOMMAND(_nickname, _command));
    }
}

std::unordered_multimap<WhichType, std::string> CommandRunner::splitTargets(std::string target)
{
    std::unordered_multimap<WhichType, std::string> targets;
    std::istringstream iss(target);
    std::string tmp;
    WhichType type;

    for (int i = 0; i < MAXTARGETS; i++) {
        if (!std::getline(iss, tmp, ',')) {
            break;
        }
        if (tmp[0] == CHANTYPES[0] || tmp[0] == CHANTYPES[1]) {
            type = static_cast<WhichType>(CHANNEL);
        }
        else {
            type = static_cast<WhichType>(NICKNAME);
        }
        targets.insert(std::make_pair(type, tmp));
    }
    return targets;
}

bool CommandRunner::validateParams(size_t min, size_t max,
                                   std::array<ParamType, MAX_PARAMS> pattern)
{
    if (_params.size() < min) {
        if (_command == "NICK")
            sendToClient(_clientFd, ERR_NONICKNAMEGIVEN(_nickname));
        else
            sendToClient(_clientFd, ERR_NEEDMOREPARAMS(_nickname, _command));
        return false;
    }

    if (_params.size() > max) {
        // truncate silently
        _params.resize(max);
    }

    // Validate each parameter according to the pattern
    for (size_t i = 0; i < _params.size(); i++) {
        std::string &param = _params[i];

        switch (pattern[i]) {
        case VAL_NICK:
            if (!IRCValidator::isValidNickname(_clientFd, _nickname, param)) {
                return false;
            }
            break;

        case VAL_CHAN:
            if (!IRCValidator::isValidChannelName(_clientFd, param)) {
                return false;
            }
            break;

        case VAL_TOPIC:
            if (!IRCValidator::isValidTopic(_clientFd, _nickname, param)) {
                return false;
            }
            break;

        case VAL_USER:
            if (!IRCValidator::isValidUsername(_clientFd, _nickname, param)) {
                return false;
            }
            break;

        case VAL_KEY:
            if (!IRCValidator::isValidChannelKey(_clientFd, _nickname, param)) {
                return false;
            }
            break;

        case VAL_PASS:
            if (!IRCValidator::isValidServerPassword(param)) {
                return false;
            }
            break;
        case VAL_TARGET:
            _targets = splitTargets(_params[0]);
            _message = _params[1];
            if (!IRCValidator::isValidTarget(_targets, _clientFd, _nickname)) {
                return false;
            }
            break;
        case VAL_REAL:
            if (!IRCValidator::isValidText(_clientFd, _nickname, param)) {
                return false;
            }
            break;
        case VAL_NONE:
            // No validation needed
            break;

        case VAL_TEXT:
            if (!IRCValidator::isValidText(_clientFd, _nickname, param)) {
                return false;
            }
            break;

        default:
            break;
        }
    }

    return true;
}

bool CommandRunner::nickNotFound(std::string &target)
{
    if (!_clients.nickExists(target)) {
        sendToClient(_clientFd, ERR_NOSUCHNICK(_nickname, target));
        return true;
    }
    return false;
}

bool CommandRunner::nickNotInChannel(Channel &channel, std::string &target)
{
    if (!channel.isOnChannel(_clients.getByNick(target))) {
        sendToClient(_clientFd, ERR_USERNOTINCHANNEL(_nickname, target, channel.getName()));
        return true;
    }
    return false;
}

bool CommandRunner::nickInUse(std::string &target)
{
    if (_clients.nickExists(target)) {
        sendToClient(_clientFd, ERR_NICKNAMEINUSE(_nickname, target));
        return true;
    }
    return false;
}

bool CommandRunner::channelNotFound(std::string &channel)
{
    if (!_channels.channelExists(channel)) {
        sendToClient(_clientFd, ERR_NOSUCHCHANNEL(_nickname, channel));
        return true;
    }
    return false;
}

bool CommandRunner::channelInUse(std::string &channel)
{
    if (_channels.channelExists(channel))
        return true;
    return false;
}

void CommandRunner::initCommandMap()
{
    _commandRunners["NICK"] = &CommandRunner::nick;
    _commandRunners["PASS"] = &CommandRunner::pass;
    _commandRunners["USER"] = &CommandRunner::user;
    _commandRunners["CAP"] = &CommandRunner::cap;
    // _commandRunners["LUSERS"] = &CommandRunner::lusers;
    _commandRunners["MOTD"] = &CommandRunner::motd;
    _commandRunners["QUIT"] = &CommandRunner::quit;
    _commandRunners["JOIN"] = &CommandRunner::join;
    _commandRunners["PART"] = &CommandRunner::part;
    _commandRunners["MODE"] = &CommandRunner::mode;
    _commandRunners["TOPIC"] = &CommandRunner::topic;
    _commandRunners["INVITE"] = &CommandRunner::invite;
    _commandRunners["PING"] = &CommandRunner::ping;
    _commandRunners["PONG"] = &CommandRunner::pong;
    _commandRunners["KICK"] = &CommandRunner::kick;
    _commandRunners["PRIVMSG"] = &CommandRunner::privmsg;
    _commandRunners["NOTICE"] = &CommandRunner::notice;
    _commandRunners["WHO"] = &CommandRunner::silentIgnore;
    // _commandRunners["WHOIS"] = &CommandRunner::whois;}
}

bool CommandRunner::canCompleteRegistration()
{
    return !_client.getIsRegistered() && _client.getNickname() != "*" &&
           !_client.getUsername().empty();
}

void CommandRunner::completeRegistration()
{
    _client.setIsRegistered(true);
    sendWelcome();
}

bool CommandRunner::tryRegisterClient()
{
    if (_client.getIsRegistered())
        return false;

    if (canCompleteRegistration()) {
        completeRegistration();
        return true;
        _channels.clearNickHistory(_client.getNickname());
    }

    return false;
}

void CommandRunner::sendWelcome()
{
    // Send the welcome messages
    sendToClient(_clientFd, RPL_WELCOME(_nickname));
    sendToClient(_clientFd, RPL_YOURHOST(_nickname));
    sendToClient(_clientFd, RPL_CREATED(_nickname, Server::getInstance().getCreatedTime()));
    sendToClient(_clientFd, RPL_MYINFO(_nickname));
    sendToClient(_clientFd, RPL_ISUPPORT(_nickname));
    motd();
}
