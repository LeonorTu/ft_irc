#pragma once

#include <string>

class IRCValidator
{
public:
    static bool isValidNickname(int clientFdconst, const std::string &sourceNick,
                                const std::string &requestedNick);
    static bool isValidUsername(int clientFd, std::string &nickname, std::string &username);
    bool isValidRealname(int clientFd, std::string &nickname, std::string &realname);
    static bool isValidChannelName();
    static bool isValidChannelMode();
    static bool isValidServerPassword();

private:
};
