#include <CommandRunner.hpp>
#include <Error.hpp>

void CommandRunner::privmsg()
{
    std::array<ParamType, MAX_PARAMS> pattern = {VAL_TARGET, VAL_TEXT};
    if (!validateParams(2, 2, pattern))
        return;

    for (auto &[type, target] : _targets) {
        if (type == CHANNEL) {
            if (!_channels.channelExists(target)) {
                sendToClient(_clientFd, ERR_NOSUCHCHANNEL(_nickname, target));
                continue;
            }
            Channel &channel = _channels.getChannel(target);
            channel.broadcastToOthers(_client,
                                      PRIVMSG(_client.getUserHost(), channel.getName(), _message));
        }
        else if (type == NICKNAME) {
            if (!_clients.nickExists(target)) {
                sendToClient(_clientFd, ERR_NOSUCHNICK(_nickname, target));
                continue;
            }
            Client &targetClient = _clients.getByNick(target);
            sendToClient(targetClient.getFd(),
                         PRIVMSG(_client.getUserHost(), targetClient.getNickname(), _message));
        }
    }
}
