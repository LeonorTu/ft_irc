#pragma once

#include <string>
#include <vector>
#include <sstream>

struct message {
    std::string prefix;
    std::string command;
    std::vector<std::string> parameters;
	message(const std::string &rawMessage);
};
