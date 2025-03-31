#pragma once
#include "IRCBot.hpp"
#include <string>
#include <cstring>
#include <queue>
#include <mutex>

class GeminiBot : public IRCBot
{
public:
    GeminiBot(const std::string &server, int port, const std::string &password,
              const std::string &nickname, const std::string &api_key);
    virtual ~GeminiBot();

    virtual void handleMessage(const std::string &message) override;

private:
    std::string _api_key;
    std::queue<std::string> _messageQueue;
    std::mutex _queueMutex;
    static const int API_TIMEOUT = 5;

    // API interaction
    std::string makeApiRequest(const std::string &prompt);
    bool connectToApi(int &sock);
    std::string sendApiRequest(int sock, const std::string &prompt);
    std::string parseApiResponse(const std::string &response);
    std::string generateGreeting(const std::string &username);

    // IRC helper functions
    std::string getNickname() const
    {
        return _nickname;
    }
    void sendMessage(const std::string &channel, const std::string &message);
    void processJoinMessage(const std::string &message);
    std::string parseNicknameFromJoin(const std::string &joinMessage);
    void queueMessage(const std::string &channel, const std::string &message);
};
