#pragma once

#include <CommandProcessor.hpp>
#include <Server.hpp>
#include <ClientIndex.hpp>
#include <Client.hpp>
#include <ChannelManager.hpp>
#include <Channel.hpp>
#include <IRCValidator.hpp>
#include <MessageParser.hpp>
#include <responses.hpp>
#include <vector>

enum ParamType
{
    NICK,
    USER,
    REAL,
    CHAN,
    MODE,
    KEY,
    PASS,
    NOVAL
};

class CommandRunner
{
public:
    CommandRunner(const MessageParser::CommandContext &ctx);
    bool validateCommandAccess();
    void execute();
    void nick();
    void user();
    void pass();
    void join();
    void part();
    void topic();
    void quit();
    void cap();
    void silentIgnore();

private:
    Server &_server;
    ClientIndex &_clients;
    ChannelManager &_channels;

    std::string _command;
    Client &_client;
    int _clientFd;
    std::string _nickname;
    std::string _messageSource;
    std::vector<std::string> _params;

    bool validateParams(size_t min, size_t max, std::array<ParamType, MAX_PARAMS> pattern);

    static bool _mapInitialized;
    static void initCommandMap();
    static std::unordered_map<std::string, void (CommandRunner::*)()> _commandRunners;

    bool canCompleteRegistration();
    void completeRegistration();
    bool tryRegisterClient();
    void sendWelcome();
};
