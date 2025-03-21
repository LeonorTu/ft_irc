#include <CommandRunner.hpp>

void CommandRunner::cap()
{
    if (_params.empty() || _params[0] != "LS")
        return;

    sendToClient(_clientFd, "CAP * LS :");
}
