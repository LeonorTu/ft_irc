#include <CommandRunner.hpp>

void CommandRunner::nick()
{
    std::array<ParamType, MAX_PARAMS> pattern = {VAL_NICK};
    if (!validateParams(1, 1, pattern))
        return;

    std::string &newNickname = _params[0];

    if (nickInUse(newNickname))
        return;

    _client.setNickname(newNickname);
    if (tryRegisterClient())
        _clients.addNick(_clientFd);
}
