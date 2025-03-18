#pragma once

#include <string>

class IRCValidator
{
public:
    static bool isValidNickname(int clientFdconst, const std::string &sourceNick,
                                const std::string &requestedNick);
    static bool isValidUsername(int clientFd, const std::string &nickname, std::string &username);
    static bool isValidRealname(int clientFd, const std::string &nickname,
                                const std::string &realname);
    static bool isValidChannelName(int clientFd, const std::string &channelName);
    static bool isValidTopic(int clientFd, const std::string &nickname,
                             const std::string &realname);
    static bool isValidChannelMode();
    static bool isValidServerPassword();
    static bool isValidChannelKey();

private:
};
