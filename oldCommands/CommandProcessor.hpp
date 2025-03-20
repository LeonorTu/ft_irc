#pragma once
#include <string>
#include <iostream>
#include <sstream>
#include <unordered_map>
#include <functional>
#include <Client.hpp>
#include <chrono>

class CommandProcessor
{
public:
    // Constructor takes client (maybe change to just nickname of the sender?) and raw command
    // string
    CommandProcessor();
    ~CommandProcessor() = default;

    // Command parameters struct - contains all data needed by handlers
    struct CommandContext
    {
        // Command identification
        int clientFd;
        bool isRegistered;
        std::string nickname;

        std::string source;
        std::string command;
        std::vector<std::string> params;
    };
    const CommandContext &getContext() const;
    // const std::string &getCommand() const;
    void executeCommand(Client &client);
    void parseCommand(Client &client, const std::string &rawString);

private:
    // Parsed command context
    CommandContext _context;
    // std::string _command;

    std::unordered_map<std::string, std::function<void(const CommandContext &)>> _commandHandlers;

    // Private methods
    void setupCommandHandlers();
    void clearCommand();
};
