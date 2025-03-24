#include <CommandRunner.hpp>

void CommandRunner::kick()
{
    std::array<ParamType, MAX_PARAMS> pattern = {VAL_CHAN};
    if (!validateParams(2, 3, pattern))
        return;
    std::string channelName = _params[0];
    std::string targetNicknames = _params[1];
    std::string reason = (_params.size() > 2) ? _params[2] : "No reason given";

    if (!_channels.channelExists(channelName))
    {
        sendToClient(_clientFd, ERR_NOSUCHCHANNEL(_nickname, channelName));
        return;
    }
    Channel &channel = _channels.getChannel(channelName);
    std::istringstream ss(targetNicknames);
    std::string targetNickname;
    while (std::getline(ss, targetNickname, ',')) {
        if (nickNotFound(targetNickname))
            continue;
        Client &targetClient = _clients.getByNick(targetNickname);
        channel.kick(_client, targetClient, reason);
    }
}