#include <CommandRunner.hpp>

void CommandRunner::quit()
{
    std::string reason = _params.empty() ? "" : _params[0];
    _client.forceQuit(reason);
}
