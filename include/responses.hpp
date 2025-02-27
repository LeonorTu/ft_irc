#pragma once

#include <string>

inline void sendToClient(int fd, std::string msg)
{
    send(fd, msg.c_str(), msg.length(), 0);
}

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

// channel Errors
inline std::string ERR_NEEDMOREPARAMS(const std::string &client, const std::string &command)
{
    return "461 " + client + " " + command + " :Not enough parameters";
}

inline std::string ERR_NOSUCHCHANNEL(const std::string &client, const std::string &channel)
{
    return "403 " + client + " " + channel + " :No such channel";
}

inline std::string ERR_BADCHANNELKEY(const std::string &client, const std::string &channel)
{
    return "475 " + client + " " + channel + " :Cannot join channel (+k) - bad key";
}

inline std::string ERR_INVITEONLYCHAN(const std::string &client, const std::string &channel)
{
    return "473 " + client + " " + channel + " :Cannot join channel (+i) - invite only";
}

// Topic replies
inline std::string RPL_TOPIC(const std::string &client, const std::string &channel, const std::string &topic)
{
    return "332 " + client + " " + channel + " :" + topic;
}

inline std::string RPL_TOPICWHOTIME(const std::string &client, const std::string &channel, const std::string &who,
                                    const std::string &time)
{
    return "333 " + client + " " + channel + " " + who + " " + time;
}

inline std::string RPL_NAMREPLY(const std::string &client, const std::string &channel, const std::string &names)
{
    return "353 " + client + " = " + channel + " :" + names;
}

inline std::string RPL_ENDOFNAMES(const std::string &client, const std::string &channel)
{
    return "366 " + client + " " + channel + " :End of /NAMES list";
}
