#pragma once

#include <string>

class IRCBot
{
public:
    IRCBot(const std::string &server, int port, const std::string &password,
           const std::string &nickname, const std::string &channel);
    virtual ~IRCBot();

    // Connection
    bool connect();
    void disconnect();
    bool isConnected() const;

    // Core IRC
    bool processServerMessages();
    void sendMessage(const std::string &message);

    // Virtual bot-specific message processing
    virtual void handleMessage(const std::string &message);

protected:
    bool reconnect();
    void sendRaw(const std::string &command);

    std::string _server;
    int _port;
    std::string _password;
    std::string _nickname;
    std::string _channel;

    int _socket;
    bool _connected;
};
