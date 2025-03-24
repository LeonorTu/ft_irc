#include <CommandRunner.hpp>

void CommandRunner::mode()
{
    std::string target = _params[0];
    if (CHANTYPES.find(target[0]) == std::string::npos)
        return;

    if (!_channels.channelExists(target)) {
        sendToClient(_clientFd, ERR_NOSUCHCHANNEL(_client.getNickname(), target));
        return;
    }
    Channel &channel = _channels.getChannel(target);

    std::string modeString = (_params.size() > 1) ? _params[1] : "";
    std::vector<std::string> params;
    for (size_t i = 2; i < _params.size(); ++i) {
        params.push_back(_params[i]);
    }

    if (modeString.empty())
        channel.printModes(_client);

    processModeString(channel, modeString, params);
}

bool CommandRunner::needsParameter(char mode, bool adding)
{
    if (mode == 'o')
        return true; // Both +o and -o need parameters
    if (adding && (mode == 'k' || mode == 'l'))
        return true; // +k and +l need parameters

    return false;
}

void CommandRunner::processModeString(Channel &channel, const std::string &modeString,
                                      const std::vector<std::string> &params)
{
    bool adding = true;
    size_t paramIndex = 0;
    for (char mode : modeString) {
        if (mode == '+') {
            adding = true;
            continue;
        }
        else if (mode == '-') {
            adding = false;
            continue;
        }

        if (std::string("iklot").find(mode) == std::string::npos) {
            continue; // Skip this mode character
        }

        if (needsParameter(mode, adding)) {
            if (paramIndex >= params.size()) {
                sendToClient(_clientFd, ERR_NEEDMOREPARAMS(_client.getNickname(), "MODE"));
                return;
            }
            std::string param = params[paramIndex++];
            if (mode == 'k' &&
                (param.empty() || !IRCValidator::isValidChannelKey(_clientFd, _nickname, param))) {
                return;
            }
            if (mode == 'o' && (nickNotFound(param) || nickNotInChannel(channel, param))) {
                return;
            }
            if (mode == 'l' && !IRCValidator::isValidChannelLimit(param)) {
                return;
            }
            channel.setMode(_client, adding, mode, param);
        }
        else {
            channel.setMode(_client, adding, mode);
        }
    }
}