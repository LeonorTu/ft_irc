#include <CommandProcessor.hpp>
#include <commandHandlers.hpp>
#include <responses.hpp>
#include <Client.hpp>

// #include "../../include/CommandProcessor.hpp"
CommandProcessor::CommandProcessor()
    : _command("")
    , _context({})
{
    setupCommandHandlers();
}

const CommandProcessor::CommandContext &CommandProcessor::getContext() const
{
    return _context;
}

const std::string &CommandProcessor::getCommand() const
{
    return _command;
}

void ignoreTag(std::istringstream &iss)
{
    if (iss.peek() == '@') {
        std::string tagIgnore;
        iss.get(); // skip the @
        std::getline(iss, tagIgnore, ' ');
        iss >> std::ws;
    }
}

void checkSource(std::istringstream &iss, CommandProcessor::CommandContext &ctx)
{
    if (iss.peek() == ':') {
        iss.get(); // skip the :
        std::getline(iss, ctx.source, ' ');
    }
}

void storeCommand(std::istringstream &iss, std::string &_command)
{
    // automatically store the first word as the command (ignoring leading whitespace)
    if (!(iss >> _command)) {
        std::cerr << "Error storing command" << std::endl;
        return;
    }
}

void param(std::istringstream &iss, CommandProcessor::CommandContext &ctx)
{
    std::string param;
    while (true) {
        iss >> std::ws;
        if (iss.peek() == ':') {
            std::string tail;
            iss.get(); // skip the :
            std::getline(iss, tail);
            ctx.params.push_back(tail);
            break;
        }
        if (!(iss >> param))
            break;
        ctx.params.push_back(param);
    }
}

void CommandProcessor::parseCommand(Client &client, const std::string &rawString)
{
    clearCommand();
    logMessage(client.getFd(), rawString, false);
    if (rawString.empty())
        return;
    std::istringstream iss(rawString);
    iss >> std::ws; // skip whitespace

    _context.clientFd = client.getFd();
    ignoreTag(iss);
    checkSource(iss, _context);
    storeCommand(iss, _command);
    param(iss, _context);
    executeCommand(client);
}

void CommandProcessor::executeCommand(Client &client)
{
    // Check if the command exists in our handlers map
    auto it = _commandHandlers.find(_command);

    if (it != _commandHandlers.end()) {
        // Call the function with the command context
        it->second(_context);
    }
    else {
        // should i need to return a message here that this cmd does not exist with 421?
        sendToClient(_context.clientFd, ERR_UNKNOWNCOMMAND(client.getNickname(), _command));
    }
}

void CommandProcessor::setupCommandHandlers()
{
    _commandHandlers["NICK"] = nick;
    _commandHandlers["PASS"] = pass;
    _commandHandlers["USER"] = user;
    _commandHandlers["CAP"] = silentIgnore;
    // _commandHandlers["LUSERS"] = lusers;
    // _commandHandlers["MOTD"] = motd;
    // _commandHandlers["QUIT"] = quit;
    // _commandHandlers["JOIN"] = join;
    // _commandHandlers["PART"] = part;
    // _commandHandlers["MODE"] = mode;
    // _commandHandlers["TOPIC"] = topic;
    // _commandHandlers["INVITE"] = invite;
    // _commandHandlers["KICK"] = kick;
    // _commandHandlers["PING"] = ping;
    // _commandHandlers["PONG"] = pong;
    // _commandHandlers["PRIVMSG"] = privmsg;
    // _commandHandlers["NOTICE"] = notice;
    // _commandHandlers["WHO"] = who;
    // _commandHandlers["WHOIS"] = whois;
}

void CommandProcessor::clearCommand()
{
    _context = {};
    _command = "";
}

//////////////////////////TESTING//////////////////////////
// int main()
// {
//     std::cout << "Testing parsing" << std::endl;
//     std::string test[6]= {
//         ":dan!d@localhost PRIVMSG #chan :Hey!",
//         ":dan!d@localhost PRIVMSG #chan ::-) hello",
//         "CAP REQ :sasl message-tags foo",
//         "@id=234AB :dan!d@localhost PRIVMSG #chan :Hey what's up!",
//         ":irc.example.com CAP LS * :multi-prefix extended-join sasl",
//         "@tag1=value1;tag2=value2 :nick!user@host PRIVMSG #channel :Hello world!"
//     };

//     for(int clientFd = 0; clientFd < 6; clientFd++)
//     {
//         std::cout << "TEST " << clientFd << " : " << test[clientFd] << std::endl;
//         CommandProcessor testing(clientFd, test[clientFd]);
//     }
//     return 0;
// }
