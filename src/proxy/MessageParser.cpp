#include <MessageParser.hpp>
#include <CommandRunner.hpp>
#include <responses.hpp>
#include <Client.hpp>
#include <Error.hpp>

MessageParser::MessageParser(int clientFd, const std::string &rawString)
    : _context({})
    , _clientFd(clientFd)
    , _rawString(rawString)
{}

const MessageParser::CommandContext &MessageParser::getContext() const
{
    return _context;
}

const std::string &MessageParser::getCommand() const
{
    return _context.command;
}

void MessageParser::executeCommand()
{
    CommandRunner runner(_context);
    runner.execute();
}

void MessageParser::parseCommand()
{
    logMessage(_clientFd, _rawString, false);
    _context.clientFd = _clientFd;
    if (_rawString.empty())
        return;
    std::istringstream iss(_rawString);
    iss >> std::ws; // skip whitespace

    ignoreTag(iss);
    checkSource(iss);
    storeCommand(iss);
    param(iss);

    executeCommand();
}

void MessageParser::ignoreTag(std::istringstream &iss)
{
    if (iss.peek() == '@') {
        std::string tagIgnore;
        iss.get(); // skip the @
        std::getline(iss, tagIgnore, ' ');
        iss >> std::ws;
    }
}

void MessageParser::checkSource(std::istringstream &iss)
{
    if (iss.peek() == ':') {
        iss.get(); // skip the :
        std::getline(iss, _context.source, ' ');
    }
}

void MessageParser::storeCommand(std::istringstream &iss)
{
    if (!(iss >> _context.command)) {
        throw MessageError("Error storing command");
    }
}

void MessageParser::param(std::istringstream &iss)
{
    std::string param;
    while (true) {
        iss >> std::ws;
        if (iss.peek() == ':') {
            std::string tail;
            iss.get(); // skip the :
            std::getline(iss, tail);
            _context.params.push_back(tail);
            // std::cout << "Param: " << tail << std::endl;
            break;
        }
        if (!(iss >> param))
            break;
        _context.params.push_back(param);
    }
}
