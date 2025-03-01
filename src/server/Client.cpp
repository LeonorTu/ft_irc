#include <Client.hpp>
#include <channel.hpp>

Client::Client(int fd, std::string &ip)
    : fd(fd)
    , nickname("*")
    , ip(ip)
    , isRegistered(false)
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

void Client::setNickname(const std::string &newNickname)
{
    nickname = newNickname;
}

void Client::registerUser()
{
    isRegistered = true;
}

const bool Client::getIsRegistered() const
{
    return isRegistered;
}

void Client::untrackChannel(Channel *channel)
{
    this->myChannels.erase(channel->getName());
}

void Client::trackChannel(Channel *channel)
{
    this->myChannels[channel->getName()] = channel;
}

bool Client::isOnChannel(Channel *channel)
{
    return this->myChannels.find(channel->getName()) != this->myChannels.end();
}
