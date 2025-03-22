#pragma once

#include <string>
#include <vector>
#include <memory>
#include <unordered_map>

class Channel;
class Client;

class ChannelManager
{
public:
    ChannelManager();
    ~ChannelManager();

    bool channelExists(const std::string &name) const;
    void createChannel(const std::string &name, Client &creator);
    void removeChannel(const std::string &name);
    Channel &getChannel(const std::string &name) const;
    void rmEmptyChannels();

private:
    std::unordered_map<std::string, std::unique_ptr<Channel>> _channels;

    // ascii casemapping, Defines the characters a to z
    // to be considered the lower-case equivalents of the characters A to Z only.
    std::string caseMapped(const std::string &name) const;
};
