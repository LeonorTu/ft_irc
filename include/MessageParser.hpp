#pragma once
#include <string>
#include <iostream>
#include <sstream>
#include <vector>

class MessageParser
{
public:
    // Constructor takes client (maybe change to just nickname of the sender?) and raw command
    // string
    MessageParser(int clientFd, const std::string &rawString);
    ~MessageParser() = default;

    // Command parameters struct - contains all data needed by handlers
    struct CommandContext
    {
        // Command identification
        int clientFd;
        std::string command;
        std::string source;
        std::vector<std::string> params;
    };
    void parseCommand();

    // getters
    const CommandContext &getContext() const;
    const std::string &getCommand() const;

private:
    // Parsed command context
    CommandContext _context;
    int _clientFd;
    const std::string &_rawString;

    // Private methods
    void executeCommand();
    void ignoreTag(std::istringstream &iss);
    void checkSource(std::istringstream &iss);
    void storeCommand(std::istringstream &iss);
    void param(std::istringstream &iss);
};
