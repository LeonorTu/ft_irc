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
    if (channelExists(caseMapped(name))) {
        _channels.erase(caseMapped(name));
        std::cout << "removed channel " << name << std::endl;
    }
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
        std::cout << "Channel '" << name << "' not found" << std::endl;
        throw std::out_of_range("Channel '" + name + "' not found");
    }
    return *it->second;
}

void ChannelManager::rmEmptyChannels()
{
    std::vector<std::string> channelsToRemove;
    for (auto &[name, channel] : _channels) {
        if (channel->isEmpty()) {
            channelsToRemove.push_back(name);
        }
    }

    for (const auto &name : channelsToRemove) {
        removeChannel(name);
    }
}

void ChannelManager::clearNickHistory(const std::string &nick)
{
    for (const auto &[_, channel] : _channels) {
        channel->eraseNickHistory(nick);
    }
}

void ChannelManager::forEachChannel(std::function<void(Channel &)> callback)
{
    // Copy all names first
    std::vector<std::string> chans;
    for (const auto &[channel, _] : _channels) {
        chans.push_back(channel);
    }

    // Then iterate through the copy
    for (std::string &channel : chans) {
        auto it = _channels.find(channel);
        if (it != _channels.end()) {
            callback(*it->second);
        }
    }
}
