#pragma once

#include <string>
#include <vector>
#include <sstream>
#include <unordered_map>
#include <functional>

struct message {
    std::string prefix;
    std::string command;
    std::vector<std::string> parameters;
    message(const std::string &rawMessage);
    std::unordered_map<std::string, std::function<void(void)>> commands;
};
