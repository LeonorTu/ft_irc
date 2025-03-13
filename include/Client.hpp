#pragma once

#include <iostream>
#include <string>
#include <netinet/in.h>
#include <unordered_map>

class Channel;
class Client
{
public:
    Client(int fd);
    ~Client();

    // getters
    const int getFd() const;
    const std::string &getNickname() const;
    const std::string &getIP() const;
    std::string &getUsername();
    void setUsername(const std::string username);
    void setNickname(const std::string &newNickname);
    void setIp(const std::string &ip);
    void registerUser();
    const bool getIsRegistered() const;
    std::string &getMessageBuf();
    void setIsRegistered(bool registered);
    void setPasswordVerified(bool verified);
    void untrackChannel(Channel *channel);
    void trackChannel(Channel *channel);
    bool isOnChannel(Channel *channel);

private:
    int _fd;
    std::string _messageBuf;
    std::string _username;
    bool _passwordVerified;
    bool _isRegistered;
    std::string _nickname;
    std::string _ip;
    std::unordered_map<std::string, Channel *> _myChannels;
};
