#include <ChannelManager.hpp>
#include <responses.hpp>
#include <Client.hpp>
#include <Channel.hpp>

ChannelManager::ChannelManager()
{}

ChannelManager::~ChannelManager()
{
    _channels.clear();
    std::cout << "channels cleared" << std::endl;
}

bool ChannelManager::channelExists(const std::string &name) const
{
    return _channels.find(caseMapped(name)) != _channels.end();
}

void ChannelManager::createChannel(const std::string &name, Client &creator)
{
    auto inserted = _channels.emplace(caseMapped(name), std::make_unique<Channel>(name, creator));
    bool successful = inserted.second;
    if (!successful) {
        std::cerr << "Channel creation failed" << std::endl;
    }
}

void ChannelManager::removeChannel(const std::string &name)
{
    if (channelExists(caseMapped(name)))
        _channels.erase(caseMapped(name));
}

std::string ChannelManager::caseMapped(const std::string &name) const
{
    std::string casemapped = name;
    for (char &c : casemapped) {
        c = tolower(c);
    }
    return casemapped;
}

Channel &ChannelManager::getChannel(const std::string &name) const
{
    auto it = _channels.find(caseMapped(name));
    if (it == _channels.end()) {
        throw std::out_of_range("Channel '" + name + "' not found");
    }
    return *it->second;
}
