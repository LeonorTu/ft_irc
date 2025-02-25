#include "message.hpp"

message::message(const std::string &rawMessage)
{
    if (rawMessage.empty())
        return ;

    std::istringstream stream(rawMessage);
    std::string word;

    // prefix
    if (rawMessage[0] == ':') {
        stream >> prefix;
        prefix = prefix.substr(1);
    }

    // command
    if (stream >> command) {
        // parameters
        while (stream >> word) {
            if (word[0] == ':') {
                std::string trailing;
                std::getline(stream, trailing);
                parameters.push_back(word.substr(1) + trailing);
                break;
            }
            parameters.push_back(word);
        }
    }
}