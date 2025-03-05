#include <Client.hpp>
#include <Channel.hpp>

Client::Client(int fd)
    : fd(fd)
    , nickname("*")
    , ip("")
    , isRegistered(false)
{}

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

void Client::setIp(const std::string &ip)
{
    this->ip = ip;
}

void Client::registerUser()
{
    isRegistered = true;
}

const bool Client::getIsRegistered() const
{
    return isRegistered;
}

std::string &Client::getMessageBuf()
{
    return _messageBuf;
}

void Client::setIsRegistered(bool registered)
{
    this->isRegistered = registered;
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
