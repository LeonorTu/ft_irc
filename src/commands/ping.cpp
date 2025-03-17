#include <pingPong.hpp>

bool rejectPingBeforeRegistered(const CommandProcessor::CommandContext &ctx)
{
    if (!ctx.isRegistered) {
        std::cout << "Not registered client trying to Ping" << std::endl;
        return true;
    }
    return false;
}

bool EmptyToken(const CommandProcessor::CommandContext &ctx)
{
    if (ctx.params.size() == 0) {
        sendToClient(ctx.clientFd, ERR_NEEDMOREPARAMS(ctx.nickname, ctx.command));
        std::cout << "Empty tokens from Client" << std::endl;
        return true;
    }
    return false;
}

bool MoreThanOneToken(const CommandProcessor::CommandContext &ctx)
{
    // for (size_t i = 0; i < ctx.params.size(); ++i) {
    //     std::cout << "Param " << i << ": \"" << ctx.params[i] << "\"" << std::endl;
    // }
    if (ctx.params.size() > 1) {
        std::cout << "More args than <token> for " + ctx.command + ": " + ctx.command + " <token>"
                  << std::endl;
        return true;
    }
    return false;
}

// already : will be not in param => take it as a tail
// bool onlyColon(const CommandProcessor::CommandContext &ctx)
// {
//     if (ctx.params[0] == ":") {
//         sendToClient(ctx.clientFd, ERR_NOORIGIN(ctx.nickname));
//         std::cout << "No specific : No origin" << std::endl;
//         return true;
//     }
//     return false;
// }

void RPL_PONG(const CommandProcessor::CommandContext &ctx)
{
    std::string response = ":" + SERVER_NAME + " PONG " + SERVER_NAME + " :" + ctx.params[0];
    sendToClient(ctx.clientFd, response);
}

void ping(const CommandProcessor::CommandContext &ctx)
{
    if (ctx.command != "PING")
        return;
    std::cout << "PING command received" << std::endl;
    if (rejectPingBeforeRegistered(ctx))
        return;
    if (EmptyToken(ctx))
        return;
    if (MoreThanOneToken(ctx))
        return;
    RPL_PONG(ctx);
}