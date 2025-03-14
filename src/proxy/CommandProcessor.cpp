#include <CommandProcessor.hpp>
#include <commandHandlers.hpp>

// #include "../../include/CommandProcessor.hpp"

CommandProcessor::CommandProcessor(int clientFd, const std::string &rawString)
    : _context({clientFd, "", {}}), _command(""), _commandHandlers({})
{
    parseCommand(rawString);
    // setupCommandHandlers();
}

CommandProcessor::~CommandProcessor(){}

void ignoreTag(std::istringstream &iss)
{
    if (iss.peek() == '@') {
        std::string tagIgnore;
        iss.get(); //skip the @
        std::getline(iss, tagIgnore, ' ');
        iss >> std::ws; 
    }
}

void checkSource(std::istringstream &iss, CommandProcessor::CommandContext &ctx)
{
    if (iss.peek() == ':') {
        iss.get(); //skip the :
        std::getline(iss, ctx.source, ' ');
    }
}

void storeCommand(std::istringstream &iss, std::string &_command)
{
    //automatically store the first word as the command (ignoring leading whitespace)
    if (!(iss >> _command)) {
        std::cerr << "Error storing command" << std::endl;
        return;
    }
}

void param(std::istringstream &iss, CommandProcessor::CommandContext &ctx)
{
    std::string param;
    while (iss >> param) {
        if (param[0] == ':') 
        {
            std::string tail;
            std::getline(iss, tail);
            ctx.params.push_back(tail);
            break;
        }
        ctx.params.push_back(param);
    }
}

void  CommandProcessor::parseCommand(const std::string &rawString)
{
    if (rawString.empty())
        return;
    std::istringstream iss(rawString);
    iss >> std::ws; //skip whitespace
    // std::cout << "here is " << iss.str() << std::endl;
    ignoreTag(iss);
    checkSource(iss, _context);
    storeCommand(iss, _command);
    param(iss, _context);

    std::cout << "Source : " << _context.source << std::endl;
    std::cout << "Command : " << _command << std::endl;
    int count = 0;
    for (const auto &param : _context.params){
        std::cout << "Param[" << count << "] :" 
        << param << std::endl; 
        count++;
    }
}



void CommandProcessor::executeCommand()
{
    // Check if the command exists in our handlers map
    auto it = _commandHandlers.find(_command);

    if (it != _commandHandlers.end()) {
        // Call the function with the command context
        it->second(_context);
    }
    else {
        std::cerr << "There is no such command: " << _command << std::endl;
    }
}

// void CommandProcessor::setupCommandHandlers()
// {
//     _commandHandlers["NICK"] = nick;
//     _commandHandlers["PASS"] = pass;
//     _commandHandlers["USER"] = user;
//     // _commandHandlers["LUSERS"] = lusers;
//     // _commandHandlers["MOTD"] = motd;
//     // _commandHandlers["QUIT"] = quit;
//     // _commandHandlers["JOIN"] = join;
//     // _commandHandlers["PART"] = part;
//     // _commandHandlers["MODE"] = mode;
//     // _commandHandlers["TOPIC"] = topic;
//     // _commandHandlers["INVITE"] = invite;
//     // _commandHandlers["KICK"] = kick;
//     // _commandHandlers["PING"] = ping;
//     // _commandHandlers["PONG"] = pong;
//     // _commandHandlers["PRIVMSG"] = privmsg;
//     // _commandHandlers["NOTICE"] = notice;
//     // _commandHandlers["WHO"] = who;
//     // _commandHandlers["WHOIS"] = whois;
// }



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