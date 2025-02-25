#pragma once

#include <string>
#include <vector>
#include <sstream>

struct message {
    std::string prefix;
    std::string command;
    std::vector<std::string> parameters;
	int parseCommand(const std::string &rawMessage);
};
