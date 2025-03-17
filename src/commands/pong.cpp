#include <pingPong.hpp>

void pong(const CommandProcessor::CommandContext &ctx)
{
    if (ctx.command != "PONG")
        return;
    if (EmptyToken(ctx) || MoreThanOneToken(ctx))
        return;
    Server &server = Server::getInstance();
    ClientIndex &clients = server.getClients();
    Client &client = clients.getByFd(ctx.clientFd);
    client.handlePongFromClient(ctx.params[0]);
}
