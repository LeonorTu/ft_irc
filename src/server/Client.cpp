#include <Client.hpp>
#include <Channel.hpp>

Client::Client(int fd)
    : _fd(fd)
    , _messageBuf("")
    , _username("")
    , _realname("")
    , _passwordVerified(false)
    , _isRegistered(false)
    , _nickname("*")
    , _ip("")
{}

Client::~Client()
{
    std::cout << "Client destructor called" << std::endl;
}

int Client::getFd() const
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

std::string &Client::getUsername()
{
    return _username;
}
void Client::setRealname(const std::string realname)
{
    _realname = realname;
}

std::string &Client::getRealname()
{
    return _realname;
}

void Client::setUsername(const std::string username)
{
    _username = username;
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

bool Client::getIsRegistered() const
{
    return _isRegistered;
}

bool Client::getPasswordVerified() const
{
    return _passwordVerified;
}

std::string &Client::getMessageBuf()
{
    return _messageBuf;
}

std::unordered_map<std::string, Channel *> Client::getMyChannels()
{
    return _myChannels;
}

void Client::setIsRegistered(bool registered)
{
    _isRegistered = registered;
}

void Client::setPasswordVerified(bool verified)
{
    _passwordVerified = verified;
}

void Client::untrackChannel(Channel *channel)
{
    _myChannels.erase(channel->getName());
}

void Client::trackChannel(Channel *channel)
{
    _myChannels[channel->getName()] = channel;
}

bool Client::isOnChannel(Channel *channel)
{
    return _myChannels.find(channel->getName()) != this->_myChannels.end();
}

size_t Client::countChannelTypes(char type)
{
    size_t counter = 0;

    for (auto &[_, channel] : _myChannels) {
        if (channel->getName()[0] == type)
            counter++;
    }
    return counter;
}
