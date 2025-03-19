#include <CommandRunner.hpp>

void CommandRunner::sendPongResponse()
{
    std::string response = ":" + SERVER_NAME + " PONG " + SERVER_NAME + " :" + ctx.params[0];
    sendToClient(_clientFd, response);
}

void CommandRunner::ping()
{
    std::array<ParamType, MAX_PARAMS> pattern = {VAL_NONE};
    if (!validateParams(1, 1, pattern))
        return;
    sendPongResponse();
}