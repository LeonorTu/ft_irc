#pragma once

#include <string>
#include <unordered_map>

class Client;

enum ChannelMode
{
    INVITE_ONLY = 'i',
    KEY = 'k',
    LIMIT = 'l',
    PROTECTED_TOPIC = 't',
    OP = 'o'
};

class Channel
{
public:
    Channel(const std::string &name, Client &creator);
    ~Channel();
    void join(Client &client, std::string const &key = "");
    void part(Client &client, std::string const &reason);
    void quit(Client &client, std::string const &reason);
    void changeTopic(Client &client, std::string &newTopic);
    void checkTopic(Client &client);
    const std::string &getName() const;
    bool hasMode(ChannelMode mode) const;
    bool hasMode(const char mode) const;
    void setMode(Client &client, bool enable, ChannelMode mode, std::string param = "");
    bool isEmpty() const;

private:
    std::string _channelName;
    std::string _topic;
    std::string _topicAuthor;
    std::string _topicTime;
    std::unordered_map<std::string, Client *> _connectedClients;
    std::unordered_map<std::string, Client *> _ops;
    std::unordered_map<std::string, Client *> _invites;
    // itkl
    std::string _modes;
    std::string _key;
    int _userLimit;

    void broadcastMessage(const std::string &message);
    void enableMode(ChannelMode mode);
    void disableMode(ChannelMode mode);
    std::string prefixNick(Client &client);
    void sendNameReply(Client &client);
    void sendTopic(Client &client);
    bool hasOp(Client &client);
    bool isInvited(Client &client);
    bool isJoinable(Client &client, std::string key);
    bool isOnChannel(Client &client);
    void removeFromInvites(Client &client);
    void addOp(std::string &nick, const std::string &modeMsg = "");
    void removeOp(std::string &nick, const std::string &modeMsg = "");
};
