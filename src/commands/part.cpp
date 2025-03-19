#include <CommandRunner.hpp>

void CommandRunner::part()
{
    if (_params.empty()) {
        sendToClient(_clientFd, ERR_NEEDMOREPARAMS(_client.getNickname(), "PART"));
        return;
    }

    std::istringstream channelList(_params[0]);
    std::string channelName;
    std::string reason = _params.size() >= 2 ? _params[1] : "";
    while (std::getline(channelList, channelName, ',')) {
        if (!channelName.empty()) {
            if (_channels.channelExists(channelName)) {
                Channel &channel = _channels.getChannel(channelName);
                channel.part(_client, reason);
            }
            else
                sendToClient(_clientFd, ERR_NOSUCHCHANNEL(_client.getNickname(), channelName));
        }
    }
}