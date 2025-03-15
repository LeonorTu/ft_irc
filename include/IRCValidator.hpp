#pragma once

#include <string>

class IRCValidator
{
public:
    static bool isValidNickname(int clientFdconst, const std::string &sourceNick,
                                const std::string &requestedNick);
    static bool isValidUsername(int clientFd, std::string &nickname, std::string &username);
    bool IRCValidator::isValidRealname(int clientFd, std::string &nickname, std::string &realname);
    static bool isValidChannelName(int clientFd, const std::string &channelName);
    static bool isValidChannelMode(int clientFd, const std::string &mode);
    static bool isValidServerPassword(int clientFd, const std::string &password);

private:
};
