#pragma once

#include <string>
#include <sys/socket.h>

inline void sendToClient(int fd, std::string msg)
{
    msg.append("\r\n");
    send(fd, msg.c_str(), msg.length(), 0);
    std::cout << "to " << fd << ": " << msg;
}

inline std::string ERR_NONICKNAMEGIVEN(const std::string &client)
{
    return "431 " + client + " :No nickname given";
}

inline std::string JOIN(const std::string &sourceNick, const std::string &channel)
{
    return ":" + sourceNick + " JOIN " + channel;
}

inline std::string QUIT(const std::string &sourceNick, const std::string &reason)
{
    return ":" + sourceNick + " QUIT " + "Quit: :" + reason;
}

inline std::string PART(const std::string &sourceNick, const std::string &channel, const std::string &reason)
{
    return ":" + sourceNick + " PART " + channel + " :" + reason;
}

inline std::string TOPIC(const std::string &channel, const std::string &topic)
{
    return "TOPIC " + channel + " :" + topic;
}

inline std::string MODE(const std::string &sourceNick, const std::string &tar, const std::string &modeChange,
                        const std::string &args = "")
{
    return ":" + sourceNick + " MODE " + tar + " " + modeChange + " " + args;
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

inline std::string ERR_NOTONCHANNEL(const std::string &client, const std::string &channel)
{
    return "442 " + client + " " + channel + " :You're not on that channel";
}
inline std::string ERR_NEEDMOREPARAMS(const std::string &client, const std::string &command)
{
    return "461 " + client + " " + command + " :Not enough parameters";
}

inline std::string ERR_NOSUCHCHANNEL(const std::string &client, const std::string &channel)
{
    return "403 " + client + " " + channel + " :No such channel";
}

inline std::string ERR_CHANNELISFULL(const std::string &client, const std::string &channel)
{
    return "471 " + client + " " + channel + " :Cannot join channel (+l) - channel full";
}

inline std::string ERR_BADCHANNELKEY(const std::string &client, const std::string &channel)
{
    return "475 " + client + " " + channel + " :Cannot join channel (+k) - bad key";
}

inline std::string ERR_INVITEONLYCHAN(const std::string &client, const std::string &channel)
{
    return "473 " + client + " " + channel + " :Cannot join channel (+i) - invite only";
}
inline std::string ERR_CHANOPRIVSNEEDED(const std::string &client, const std::string &channel)
{
    return "482 " + client + " " + channel + " You're not channel operator";
}

// Topic replies
inline std::string RPL_NOTOPIC(const std::string &client, const std::string &channel)
{
    return "331 " + client + " " + channel + " :No topic is set";
}
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
