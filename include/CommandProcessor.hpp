#pragma once
#include <string>
#include <iostream>
#include <unordered_map>
#include <functional>

class CommandProcessor
{
public:
    // Constructor takes client (maybe change to just nickname of the sender?) and raw command
    // string
    CommandProcessor(int clientFd, const std::string &rawString);
    ~CommandProcessor();

    // Command parameters struct - contains all data needed by handlers
    struct CommandContext
    {
        // Command identification
        int clientFd;
        std::string source;
        std::vector<std::string> params;
    };
    void executeCommand();

private:
    // Parsed command context
    CommandContext _context;
    std::string _command;

    std::unordered_map<std::string, std::function<void(const CommandContext &)>> _commandHandlers;

    // Private methods
    void parseCommand(const std::string &rawString);
    void setupCommandHandlers();
};