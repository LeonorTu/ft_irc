#include <CommandRunner.hpp>
#include <Error.hpp>

void CommandRunner::notice()
{
    // std::array<ParamType, MAX_PARAMS> pattern = {VAL_NONE};
    if (_params.size() != 2)
        return;
    _targets = splitTargets(_params[0]);
    _message = _params[1];
    for (auto &[type, target] : _targets) {
        if (type == CHANNEL) {
            Channel *channel = nullptr;
            try {
                channel = &_channels.getChannel(target);
            }
            catch (const ChannelNotFound &e) {
                continue;
            }
            channel->broadcastToOthers(_client, ":" + _client.getUserHost() + " NOTICE " + target +
                                                    " :" + _message);
        }
        else if (type == NICKNAME) {
            if (!_clients.nickExists(target)) {
                continue;
            }
            Client &targetClient = _clients.getByNick(target);
            sendToClient(targetClient.getFd(),
                         ":" + _client.getUserHost() + " NOTICE " + target + " :" + _message);
        }
    }
}