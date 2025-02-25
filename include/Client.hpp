#pragma once

#include <iostream>
#include <string>
#include <netinet/in.h>

class Client {
public:
    Client(int fd, std::string &ip);
    ~Client();

    // getters
    const int &getFd() const;
    const std::string &getNickname() const;
    const std::string &getIP() const;
    const size_t &getPollIndex() const;
    // setters
    void setPollIndex(size_t index);
    void setNickname(const std::string &newNickname);

private:
    size_t pollIndex;
    int fd;
    std::string nickname;
    std::string ip;
};
