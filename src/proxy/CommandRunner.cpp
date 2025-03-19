#include <CommandRunner.hpp>
#include <unordered_set>
#include <array>

bool CommandRunner::_mapInitialized = false;
std::unordered_map<std::string, void (CommandRunner::*)()> CommandRunner::_commandRunners;

CommandRunner::CommandRunner(const MessageParser::CommandContext &ctx)
    : _server(Server::getInstance())
    , _clients(_server.getClients())
    , _channels(_server.getChannels())
    , _client(_clients.getByFd(ctx.clientFd))
    , _pingPongManager(_server.getPingPongManager())
    , _command(ctx.command)
    , _clientFd(ctx.clientFd)
    , _nickname(_client.getNickname())
    , _messageSource(ctx.source)
    , _params(ctx.params)
{
    if (!_mapInitialized)
        initCommandMap();
}

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
            if (!IRCValidator::isPrintable(_clientFd, _nickname, param, TOPICLEN)) {
                return false;
            }
            break;

        case VAL_MODE:
            if (!IRCValidator::isValidChannelMode()) {
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
            if (!IRCValidator::isValidServerPassword()) {
                return false;
            }
            break;

        case VAL_REAL:
            // Usually no validation for realname
            break;

        case VAL_NONE:
            // No validation needed
            break;

        case VAL_TEXT:
            if (!IRCValidator::isPrintable(_clientFd, _nickname, param, MSG_BUFFER_SIZE)) {
                return false;
            }
            break;

        case VAL_USERLIST:
            

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
    _commandRunners["CAP"] = &CommandRunner::silentIgnore;
    // _commandRunners["LUSERS"] = &CommandRunner::lusers;
    // _commandRunners["MOTD"] = &CommandRunner::motd;
    _commandRunners["QUIT"] = &CommandRunner::quit;
    _commandRunners["JOIN"] = &CommandRunner::join;
    _commandRunners["PART"] = &CommandRunner::part;
    // _commandRunners["MODE"] = &CommandRunner::mode;
    _commandRunners["TOPIC"] = &CommandRunner::topic;
    _commandRunners["INVITE"] = &CommandRunner::invite;
    _commandRunners["KICK"] = &CommandRunner::kick;
    // _commandRunners["PING"] = &CommandRunner::ping;
    // _commandRunners["PONG"] = &CommandRunner::pong;
    // _commandRunners["PRIVMSG"] = &CommandRunner::privmsg;
    // _commandRunners["NOTICE"] = &CommandRunner::notice;
    // _commandRunners["WHO"] = &CommandRunner::who;
    // _commandRunners["WHOIS"] = &CommandRunner::whois;}
    _mapInitialized = true;
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
}
