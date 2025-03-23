#pragma once

#include <string>
#include <vector>
#include <memory>
#include <unordered_map>
#include <functional>

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
    void clearNickHistory(const std::string &nickname);
    void forEachChannel(std::function<void(Channel &)> callback);

private:
    std::unordered_map<std::string, std::unique_ptr<Channel>> _channels;

    // ascii casemapping, Defines the characters a to z
    // to be considered the lower-case equivalents of the characters A to Z only.
    std::string caseMapped(const std::string &name) const;
};
