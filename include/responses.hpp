#pragma once

#include <string>

inline std::string ERR_NONICKNAMEGIVEN(const std::string &client)
{
    return "431 " + client + " :No nickname given";
}

inline std::string ERR_NICKNAMEINUSE(const std::string &client, const std::string &nickname)
{
    return "433 " + client + " " + nickname + " :Nickname is already in use";
}

inline std::string ERR_ERRONEUSNICKNAME(const std::string &client, const std::string &nickname)
{
    return "432 " + client + " " + nickname + " :Erroneus nickname";
}

inline std::string NICKNAMECHANGE(const std::string &old_nickname, const std::string &new_nickname)
{
    return old_nickname + " changed their nickname to " + new_nickname;
}