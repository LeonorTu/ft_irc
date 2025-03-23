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
    void invite(Client &inviter, Client &target);
    void kick(Client &kicker, Client &target, std::string const &reason);
    void changeTopic(Client &client, std::string &newTopic);
    void checkTopic(Client &client);
    const std::string &getName() const;
    const std::string &getCreatedTime();
    bool hasMode(ChannelMode mode) const;
    bool hasMode(const char mode) const;
    void setMode(Client &client, bool enable, ChannelMode mode, std::string param = "");
    void setMode(Client &client, bool enable, const char mode, std::string param = "");
    void printModes(Client &client);
    bool isEmpty() const;
    void broadcastMessage(const std::string &message);
    void broadcastToOthers(Client &client, const std::string &message);
    bool hasOp(Client &client);
    void eraseNickHistory(const std::string &nick);
    void updateNick(Client &client, const std::string &newNick);

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
    size_t _userLimit;
    std::string _createdTime;

    void enableMode(ChannelMode mode);
    void disableMode(ChannelMode mode);
    std::string prefixNick(Client &client);
    void sendNameReply(Client &client);
    void sendTopic(Client &client);
    bool isInvited(Client &client);
    bool isJoinable(Client &client, std::string key);
    bool isOnChannel(Client &client);
    void removeFromInvites(Client &client);
    void addOp(std::string &nick, const std::string &modeMsg = "");
    void removeOp(const std::string &nick, const std::string &modeMsg = "");
};
