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
    std::string &getUsername();
    std::string &getRealname();
    void setUsername(const std::string username);
    void setRealname(const std::string realname);
    void setNickname(const std::string &newNickname);
    void setIp(const std::string &ip);
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
    int getTimeForNoActivity() const;
    void quit(const std::string &reason);

private:
    int _fd;
    std::string _messageBuf;
    std::string _username;
    std::string _realname;
    bool _passwordVerified;
    bool _isRegistered;
    std::string _nickname;
    std::string _ip;
    std::unordered_map<std::string, Channel *> _myChannels;
   
    // void listClients(ClientIndex &_clients);
    std::chrono::steady_clock::time_point _lastactivityTime;
};
