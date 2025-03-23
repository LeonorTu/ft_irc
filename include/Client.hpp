#pragma once

#include <iostream>
#include <string>
#include <netinet/in.h>
#include <unordered_map>
#include <chrono>
#include <responses.hpp>

class Channel;
class Client
{
public:
    Client(int fd);
    ~Client();

    // getters
    int getFd() const;
    const std::string &getNickname() const;
    const std::string &getIP() const;
    const std::string &getUsername() const;
    const std::string &getRealname() const;
    void setUsername(const std::string username);
    void setRealname(const std::string realname);
    void setNickname(const std::string &newNickname);
    void setIp(const std::string &ip);
    const std::string &getUserHost() const;
    void updateUserHost();
    void registerUser();
    bool getIsRegistered() const;
    bool getPasswordVerified() const;
    std::string &getMessageBuf();
    void setIsRegistered(bool registered);
    void setPasswordVerified(bool verified);
    void untrackChannel(Channel *channel);
    void trackChannel(Channel *channel);
    bool isOnChannel(Channel *channel);
    size_t countChannelTypes(char type);
    std::unordered_map<std::string, Channel *> getMyChannels();
    void updateActivityTime();
    std::chrono::steady_clock::time_point getLastActivityTime() const;
    void updateMyChannelsNick(const std::string &newNick);
    // int getTimeForNoActivity() const;
    void forceQuit(const std::string &reason);
    void broadcastMyChannels(const std::string &msg);
    void markPingSent(const std::string &token);
    bool isWaitingForPong() const;
    void noPongWait();
    const std::string &getLastPingToken() const;
    int getTimeSinceLastPing() const;
    std::string getPrefixPrivmsg();

private:
    int _fd;
    std::string _messageBuf;
    std::string _username;
    std::string _realname;
    bool _passwordVerified;
    bool _isRegistered;
    std::string _nickname;
    std::string _ip;
    std::string _userHost;
    std::unordered_map<std::string, Channel *> _myChannels;

    // when you receive pong from client
    std::chrono::steady_clock::time_point _lastactivityTime;
    std::chrono::steady_clock::time_point _lastPingSentTime;
    bool _waitingForPong;
    std::string _lastPingToken;
};
