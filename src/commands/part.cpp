#include <CommandRunner.hpp>

void CommandRunner::part()
{
    std::array<ParamType, MAX_PARAMS> pattern = {};
    if (!validateParams(1, 2, pattern))
        return;
        
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