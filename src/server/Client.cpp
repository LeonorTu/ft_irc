#include <Client.hpp>
#include <Channel.hpp>

Client::Client(int fd)
    : _fd(fd)
    , _nickname("*")
    , _ip("")
    , _isRegistered(false)
{}

Client::~Client()
{
    std::cout << "Client destructor called" << std::endl;
}

const int Client::getFd() const
{
    return this->_fd;
}

const std::string &Client::getNickname() const
{
    return this->_nickname;
}

const std::string &Client::getIP() const
{
    return _ip;
}

void Client::setNickname(const std::string &newNickname)
{
    _nickname = newNickname;
}

void Client::setIp(const std::string &ip)
{
    _ip = ip;
}

void Client::registerUser()
{
    _isRegistered = true;
}

const bool Client::getIsRegistered() const
{
    return _isRegistered;
}

std::string &Client::getMessageBuf()
{
    return _messageBuf;
}

void Client::setIsRegistered(bool registered)
{
    this->_isRegistered = registered;
}

void Client::untrackChannel(Channel *channel)
{
    this->_myChannels.erase(channel->getName());
}

void Client::trackChannel(Channel *channel)
{
    this->_myChannels[channel->getName()] = channel;
}

bool Client::isOnChannel(Channel *channel)
{
    return this->_myChannels.find(channel->getName()) != this->_myChannels.end();
}
