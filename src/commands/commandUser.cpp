#include <CommandRunner.hpp>

void CommandRunner::user()
{
    std::array<ParamType, MAX_PARAMS> pattern = {USER, NOVAL, NOVAL, REAL};
    if (!validateParams(4, 4, pattern))
        return;

    std::string username = _params[0];
    std::string realname = _params[3];
    _client.setUsername(username);
    _client.setRealname(realname);

    if (tryRegisterClient())
        _clients.addNick(_clientFd);
}
