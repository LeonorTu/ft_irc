#pragma once

#include <CommandProcessor.hpp>

void nick(const CommandProcessor::CommandContext &ctx);
void pass(const CommandProcessor::CommandContext &ctx);
void user(const CommandProcessor::CommandContext &ctx);
void silentIgnore(const CommandProcessor::CommandContext &ctx);
void join(const CommandProcessor::CommandContext &ctx);
void part(const CommandProcessor::CommandContext &ctx);
void quit(const CommandProcessor::CommandContext &ctx);
void sendWelcome(int clientFd);
