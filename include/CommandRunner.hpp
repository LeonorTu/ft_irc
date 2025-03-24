#pragma once

#include <Server.hpp>
#include <ClientIndex.hpp>
#include <Client.hpp>
#include <ChannelManager.hpp>
#include <Channel.hpp>
#include <IRCValidator.hpp>
#include <MessageParser.hpp>
#include <responses.hpp>
#include <ConnectionManager.hpp>
#include <vector>
#include <PongManager.hpp>

enum ParamType
{
    VAL_NONE,
    VAL_NICK,
    VAL_USER,
    VAL_REAL,
    VAL_CHAN,
    VAL_TOPIC,
    VAL_KEY,
    VAL_PASS,
    VAL_TEXT,
    VAL_TARGET
};

class CommandRunner
{
public:
    CommandRunner(const MessageParser::CommandContext &ctx);
    void execute();

private:
    // server data
    Server &_server;
    ClientIndex &_clients;
    ChannelManager &_channels;
    Client &_client;
    PongManager &_PongManager;

    // shared pre-loads
    const std::string &_command;
    int _clientFd;
    const std::string _nickname;
    const std::string _userHost;
    const std::string &_messageSource;
    std::vector<std::string> _params;
    std::unordered_multimap<WhichType, std::string> _targets;
    std::string _message;

    // commands
    void nick();
    void user();
    void pass();
    void join();
    void part();
    void topic();
    void quit();
    void cap();
    void mode();
    void kick();
    void ping();
    void pong();
    void motd();
    void silentIgnore();
    void invite();
    void privmsg();
    void notice();

    // utils
    void leaveAllChannels();
    void handleJoinChannel(const std::string &channelName, const std::string &key);

    // validation
    bool validateCommandAccess();
    bool validateParams(size_t min, size_t max, std::array<ParamType, MAX_PARAMS> pattern);
    std::unordered_multimap<WhichType, std::string> splitTargets(std::string target);

    // common error handlers
    bool nickNotFound(std::string &nickname);
    bool nickInUse(std::string &nickname);
    bool channelNotFound(std::string &channel);
    bool channelInUse(std::string &channel);
    // static command map
    static bool _mapInitialized;
    static void initCommandMap();
    static std::unordered_map<std::string, void (CommandRunner::*)()> _commandRunners;

    // registration
    bool canCompleteRegistration();
    void completeRegistration();
    bool tryRegisterClient();
    void sendWelcome();
    void sendPongResponse();
};
