#include "Client.hpp"

Client::Client(int fd, std::string &ip) : fd(fd), nickname("Test"), ip(ip)
{
}

Client::~Client()
{
    std::cout << "Client destructor called" << std::endl;
}

const int &Client::getFd() const
{
    return this->fd;
}

const std::string &Client::getNickname() const
{
    return this->nickname;
}

const std::string &Client::getIP() const
{
    return this->ip;
}

const size_t &Client::getPollIndex() const
{
    return this->pollIndex;
}

void Client::setPollIndex(size_t index)
{
    this->pollIndex = index;
}

void Client::setNickname(const std::string &newNickname)
{
    nickname = newNickname;
}
