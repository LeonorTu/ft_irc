#include <CommandRunner.hpp>

void CommandRunner::join()
{
    if (_params.empty()) {
        sendToClient(_clientFd, ERR_NEEDMOREPARAMS(_client.getNickname(), "JOIN"));
        return;
    }

    if (_params[0] == "0") {
        leaveAllChannels();
        return;
    }

    std::istringstream channelList(_params[0]);
    std::istringstream keyList(_params.size() > 1 ? _params[1] : "");
    std::string channelName;
    std::string key;
    IRCValidator validator;
    while (std::getline(channelList, channelName, ',')) {
        if (channelName.empty()) {
            continue;
        }

        if (!validator.isValidChannelName(_clientFd, channelName)) {
            sendToClient(_clientFd, ERR_NOSUCHCHANNEL(_nickname, channelName));
            continue;
        }

        if (channelName[0] == '#' && _client.countChannelTypes('#') > REGCHANLMAX) {
            sendToClient(_clientFd, ERR_TOOMANYCHANNELS(_nickname, channelName));
            continue;
        }

        if (channelName[0] == '&' && _client.countChannelTypes('&') > LOCCHANLMAX) {
            sendToClient(_clientFd, ERR_TOOMANYCHANNELS(_nickname, channelName));
            continue;
        }

        if (!std::getline(keyList, key, ',')) {
            key.clear();
        }

        handleJoinChannel(channelName, key);
    }
}

void CommandRunner::leaveAllChannels()
{
    std::unordered_map<std::string, Channel *> channels = _client.getMyChannels();
    for (auto &pair : channels) {
        Channel *channel = pair.second;
        channel->part(_client, "Client is leaving all the channels");
    }
}

void CommandRunner::handleJoinChannel(const std::string &channelName, const std::string &key)
{
    if (_channels.channelExists(channelName)) {
        Channel &channel = _channels.getChannel(channelName);
        channel.join(_client, key);
    }
    else {
        _channels.createChannel(channelName, _client);
    }
}