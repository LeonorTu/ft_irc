#include <IRCValidator.hpp>
#include <common.hpp>
#include <regex>

bool IRCValidator::isValidNickname(const std::string &nickname)
{
    if (nickname.empty() || nickname.length() > NICKLEN)
        return false;
    std::regex nicknamePattern("^[a-zA-Z\\[\\]\\\\`_^{|}][a-zA-Z0-9\\[\\]\\\\`_^{|}-]*$");

    return std::regex_match(nickname, nicknamePattern);
}

bool IRCValidator::isValidChannelName(int clientFd, const std::string &channelName)
{
    return true;
}

bool IRCValidator::isValidUsername(const std::string &username)
{
    if (username.empty())
        return false;
    std::regex usernamePattern(R"(^[a-zA-Z0-9_-]+$)");

    return std::regex_match(username, usernamePattern);
}

bool IRCValidator::isValidChannelMode(int clientFd, const std::string &mode)
{
    return true;
}

bool IRCValidator::isValidServerPassword(int clientFd, const std::string &password)
{
    return true;
}
