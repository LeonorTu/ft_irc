#pragma once

#include <string>
#include <sys/socket.h>
#include <iostream>
#include <chrono>
#include <ctime>
#include <sstream>
#include <common.hpp>

// Time utilities
inline std::string getCurrentTime()
{
    auto now = std::chrono::system_clock::now();
    std::time_t now_time_t = std::chrono::system_clock::to_time_t(now);
    char buffer[80];
    std::strftime(buffer, sizeof(buffer), "%a %d %b %H:%M:%S %Y", std::localtime(&now_time_t));
    return std::string(buffer);
}

inline std::string getLogTimestamp()
{
    auto now = std::chrono::system_clock::now();
    std::time_t now_time_t = std::chrono::system_clock::to_time_t(now);
    char buffer[80];
    std::strftime(buffer, sizeof(buffer), "[%Y-%d-%m %H:%M:%S]", std::localtime(&now_time_t));
    return std::string(buffer);
}

// Logging function
inline void logMessage(int fd, const std::string &msg, bool outgoing = true)
{
    std::string direction = outgoing ? "to" : "from";
    std::cout << getLogTimestamp() << " " << direction << " " << fd << ": " << msg << std::endl;
}

// Client communication
inline void sendToClient(int fd, std::string msg)
{
    msg.append("\r\n");
    send(fd, msg.c_str(), msg.length(), 0);
    logMessage(fd, msg);
}

/* WELCOME MESSAGES (001-005) */
inline std::string RPL_WELCOME(const std::string &nickname)
{
    return ":" + SERVER_NAME + " 001 " + nickname + " :Welcome to " + NETWORK_NAME + " Network, " +
           nickname;
}

inline std::string RPL_YOURHOST(const std::string &nickname)
{
    return ":" + SERVER_NAME + " 002 " + nickname + " :Your host is " + SERVER_NAME +
           ", running version " + SERVER_VERSION;
}

inline std::string RPL_CREATED(const std::string &nickname, const std::string &createdTime)
{
    return ":" + SERVER_NAME + " 003 " + nickname + " :This server was created " + createdTime;
}

inline std::string RPL_MYINFO(const std::string &nickname)
{
    return ":" + SERVER_NAME + " 004 " + nickname + " " + SERVER_NAME + " " + SERVER_VERSION + " " +
           USER_MODES + " " + CHANNEL_MODES;
}

inline std::string RPL_ISUPPORT(const std::string &nickname)
{
    std::ostringstream oss;
    oss << ":" << SERVER_NAME << " 005 " << nickname << " ";
    oss << "CASEMAPPING=" << CASEMAPPING << " ";
    oss << "CHANNELLEN=" << CHANNELLEN << " ";
    oss << "CHANLIMIT=" << CHANLIMIT << " ";
    oss << "CHANTYPES=" << CHANTYPES << " ";
    oss << "CHANMODES=" << CHANMODES << " ";
    oss << "PREFIX=" << PREFIX << " ";
    oss << "MODES=" << MODES << " ";
    oss << "NICKLEN=" << NICKLEN << " ";
    oss << "TOPICLEN=" << TOPICLEN << " ";
    oss << "USERLEN=" << USERLEN << " ";
    oss << ":are supported by this server";
    return oss.str();
}

/* COMMAND RESPONSES */
inline std::string JOIN(const std::string &sourceNick, const std::string &channel)
{
    return ":" + sourceNick + " JOIN " + channel;
}

inline std::string QUIT(const std::string &sourceNick, const std::string &reason)
{
    return ":" + sourceNick + " QUIT " + "Quit: :" + reason;
}

inline std::string PART(const std::string &sourceNick, const std::string &channel,
                        const std::string &reason)
{
    return ":" + sourceNick + " PART " + channel + " :" + reason;
}

inline std::string TOPIC(const std::string &channel, const std::string &topic)
{
    return "TOPIC " + channel + " :" + topic;
}

inline std::string MODE(const std::string &sourceNick, const std::string &tar,
                        const std::string &modeChange, const std::string &args = "")
{
    return ":" + sourceNick + " MODE " + tar + " " + modeChange + " " + args;
}

inline std::string NICK(const std::string &oldNickname, const std::string &newNickname)
{
    return ":" + oldNickname + " NICK " + newNickname;
}

inline std::string INVITE(const std::string &issuer, const std::string &target,
                          const std::string &channel)
{
    return ":" + issuer + " INVITE " + target + " " + channel;
}

/* INFORMATIONAL RESPONSES (RPL_*) */
inline std::string RPL_NOTOPIC(const std::string &client, const std::string &channel)
{
    return "331 " + client + " " + channel + " :No topic is set";
}

inline std::string RPL_TOPIC(const std::string &client, const std::string &channel,
                             const std::string &topic)
{
    return "332 " + client + " " + channel + " :" + topic;
}

inline std::string RPL_TOPICWHOTIME(const std::string &client, const std::string &channel,
                                    const std::string &who, const std::string &time)
{
    return "333 " + client + " " + channel + " " + who + " " + time;
}

inline std::string RPL_INVITING(const std::string &client, const std::string &nickname,
                                const std::string &channel)
{
    return "341" + client + nickname + channel;
}

inline std::string RPL_NAMREPLY(const std::string &client, const std::string &channel,
                                const std::string &names)
{
    return "353 " + client + " = " + channel + " :" + names;
}

inline std::string RPL_ENDOFNAMES(const std::string &client, const std::string &channel)
{
    return "366 " + client + " " + channel + " :End of /NAMES list";
}

inline std::string ERR_NOSUCHNICK(const std::string &client, const std::string &nickname)
{
    return "401 " + client + " " + nickname + " :No such nick/channel";
}

inline std::string ERR_NOSUCHCHANNEL(const std::string &client, const std::string &channel)
{
    return "403 " + client + " " + channel + " :No such channel";
}

inline std::string ERR_TOOMANYCHANNELS(const std::string &client, const std::string &channel)
{
    return "405 " + client + " " + channel + " :You have joined too many channels";
}

/* ERROR RESPONSES */
inline std::string ERR_NOORIGIN(const std::string &client)
{
    return "409 " + client + " :No origin specified";
}

inline std::string ERR_UNKNOWNCOMMAND(const std::string &client, const std::string &command)
{
    return "421 " + client + " " + command + " :Unknown command";
}

inline std::string ERR_NONICKNAMEGIVEN(const std::string &client)
{
    return "431 " + client + " :No nickname given";
}

inline std::string ERR_ERRONEUSNICKNAME(const std::string &client, const std::string &nickname)
{
    return "432 " + client + " " + nickname + " :Erroneus nickname";
}

inline std::string ERR_NICKNAMEINUSE(const std::string &client, const std::string &nickname)
{
    return "433 " + client + " " + nickname + " :Nickname is already in use";
}

inline std::string ERR_NOTONCHANNEL(const std::string &client, const std::string &channel)
{
    return "442 " + client + " " + channel + " :You're not on that channel";
}
inline std::string ERR_USERONCHANNEL(const std::string &client, const std::string &target,
                                     const std::string &channel)
{
    return "443 " + client + " " + target + " " + channel + " :is already on channel";
}

inline std::string ERR_NOTREGISTERED(const std::string &client)
{
    return "451" + client + " :You have not registered";
}

inline std::string ERR_NEEDMOREPARAMS(const std::string &client, const std::string &command)
{
    return "461 " + client + " " + command + " :Not enough parameters";
}

inline std::string ERR_ALREADYREGISTERED(const std::string &client)
{
    return "462 " + client + " :You may not reregister";
}

inline std::string ERR_PASSWDMISMATCH(const std::string &client)
{
    return "464" + client + " :Password incorrect";
}

inline std::string ERR_INVALIDUSERNAME(const std::string &client, const std::string &username)
{
    return "468 " + client + " " + username + " :Invalid username format";
}

inline std::string ERR_CHANNELISFULL(const std::string &client, const std::string &channel)
{
    return "471 " + client + " " + channel + " :Cannot join channel (+l) - channel full";
}

inline std::string ERR_BADCHANNELKEY(const std::string &client, const std::string &channel)
{
    return "475 " + client + " " + channel + " :Cannot join channel (+k) - bad key";
}

inline std::string ERR_BADCHANMASK(const std::string &channel)
{
    return "476 " + channel + " :Bad Channel Mask";
}

inline std::string ERR_INVITEONLYCHAN(const std::string &client, const std::string &channel)
{
    return "473 " + client + " " + channel + " :Cannot join channel (+i) - invite only";
}

inline std::string ERR_INVALIDTEXT(const std::string &client,
                                                   const std::string &text)
{
    return "479 " + client + text + " :Invalid  "; // selfmade
}

inline std::string ERR_CHANOPRIVSNEEDED(const std::string &client, const std::string &channel)
{
    return "482 " + client + " " + channel + " :You're not channel operator";
}

inline std::string ERR_INVALIDREALNAME(const std::string &client, const std::string &realname)
{
    return "513 " + client + " " + realname + " :Invalid characters in realname";
}

inline std::string ERR_INVALIDKEY(const std::string &client, const std::string &channel)
{
    return "525 " + client + " " + channel + " :Key is not well-formed";
}
