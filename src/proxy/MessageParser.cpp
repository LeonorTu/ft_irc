#include <MessageParser.hpp>
#include <CommandRunner.hpp>
#include <responses.hpp>
#include <Client.hpp>

MessageParser::MessageParser(int clientFd, const std::string &rawString)
    : _context({})
{
    parseCommand(clientFd, rawString);
    executeCommand();
}

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

void MessageParser::parseCommand(int clientFd, const std::string &rawString)
{
    clearCommand();
    logMessage(_context.clientFd, rawString, false);
    if (rawString.empty())
        return;
    std::istringstream iss(rawString);
    iss >> std::ws; // skip whitespace

    ignoreTag(iss);
    checkSource(iss);
    storeCommand(iss);
    param(iss);
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
    // automatically store the first word as the command (ignoring leading whitespace)
    if (!(iss >> _context.command)) {
        std::cerr << "Error storing command" << std::endl;
        return;
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
            break;
        }
        if (!(iss >> param))
            break;
        _context.params.push_back(param);
    }
}

void MessageParser::clearCommand()
{
    _context = {};
}
