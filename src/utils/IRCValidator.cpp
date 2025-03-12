#include <IRCValidator.hpp>

bool IRCValidator::isValidNickname(int clientFd, const std::string &nickname)
{
    return true;
}

bool IRCValidator::isValidChannelName(int clientFd, const std::string &channelName)
{
    return true;
}

bool IRCValidator::isValidUsername(int clientFd, const std::string &username)
{
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
