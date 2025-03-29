#pragma once

#include "IRCBot.hpp"

class LogBot : public IRCBot {
public:
    LogBot(const std::string& server, int port, const std::string& password,
           const std::string& nickname, const std::string& channel);
    
    virtual void handleMessage(const std::string& sender, const std::string& message) override;
    void logMessage(const std::string& message);
};
