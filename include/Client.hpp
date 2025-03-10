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
    void setNickname(const std::string &newNickname);
    void setIp(const std::string &ip);
    void registerUser();
    const bool getIsRegistered() const;
    std::string &getMessageBuf();
    void setIsRegistered(bool registered);
    void untrackChannel(Channel *channel);
    void trackChannel(Channel *channel);
    bool isOnChannel(Channel *channel);

private:
    int fd;
    std::string _messageBuf;
    std::string nickname;
    std::string ip;
    bool isRegistered;
    std::unordered_map<std::string, Channel *> myChannels;
};
