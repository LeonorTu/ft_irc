#include <CommandRunner.hpp>

void CommandRunner::invite()
{
    std::array<ParamType, MAX_PARAMS> pattern = {VAL_NICK, VAL_CHAN};
    if (!validateParams(2, 2, pattern))
        return;

    std::string targetNickname = _params[0];
    std::string channelName = _params[1];

    if (nickNotFound(targetNickname) || channelNotFound(channelName))
        return;

    Client &targetClient = _clients.getByNick(targetNickname);
    Channel &channel = _channels.getChannel(channelName);
    channel.invite(_client, targetClient);
}
