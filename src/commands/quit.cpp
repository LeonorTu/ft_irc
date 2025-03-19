#include <CommandRunner.hpp>

void CommandRunner::quit()
{
    std::unordered_map<std::string, Channel *> channels = _client.getMyChannels();
    std::string reason = _params.empty() ? "" : _params[0];

    for (auto &pair : channels) {
        Channel *channel = pair.second;
        channel->quit(_client, reason);
    }
}