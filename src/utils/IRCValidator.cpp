#include <IRCValidator.hpp>
#include <common.hpp>
#include <regex>
#include <responses.hpp>

bool IRCValidator::isValidNickname(int clientFd, const std::string &sourceNick,
                                   const std::string &requestedNick)
{
    if (requestedNick.empty() || requestedNick.length() > NICKLEN)
        return false;
    std::regex nicknamePattern(R"(^[a-zA-Z\[\]\\`_^{|}][a-zA-Z0-9\[\]\\`_^{|}-]*$)");
    if (!std::regex_match(requestedNick, nicknamePattern)) {
        sendToClient(clientFd, ERR_ERRONEUSNICKNAME(sourceNick, requestedNick));
        return false;
    }
    return true;
}

bool IRCValidator::isValidChannelName(int clientFd, const std::string &channelName)
{
    return true;
}

bool IRCValidator::isValidUsername(int clientFd, std::string &nickname, std::string &username)
{
    if (username.empty())
        return false;
    if (username.length() > USERLEN) {
        username = username.substr(0, USERLEN);
    }
    std::regex usernamePattern(R"(^[a-zA-Z0-9_-]+$)");
    if (!std::regex_match(username, usernamePattern))
    {
        sendToClient(clientFd, ERR_INVALIDUSERNAME(nickname, username));
        return false;
    }
    return true;
}

bool IRCValidator::isValidChannelMode(int clientFd, const std::string &mode)
{
    return true;
}

bool IRCValidator::isValidServerPassword(int clientFd, const std::string &password)
{
    return true;
}
