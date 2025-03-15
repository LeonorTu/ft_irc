#pragma once

#include <CommandProcessor.hpp>

void nick(const CommandProcessor::CommandContext &ctx);
void pass(const CommandProcessor::CommandContext &ctx);
void user(const CommandProcessor::CommandContext &ctx);
void sendWelcome(int clientFd);
