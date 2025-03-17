#pragma once

#include <responses.hpp>
#include <CommandProcessor.hpp>
#include <Server.hpp>
#include <ClientIndex.hpp>
#include <Client.hpp>
#include <ConnectionManager.hpp>

void pong(CommandProcessor::CommandContext &ctx);
void ping(const CommandProcessor::CommandContext &ctx);
bool EmptyToken(const CommandProcessor::CommandContext &ctx);
bool MoreThanOneToken(const CommandProcessor::CommandContext &ctx);
bool onlyColon(const CommandProcessor::CommandContext &ctx);

// Client.cpp
// void Client::addPingToken(const std::string &token);
// void Client::handlePongFromClient(const std::string &token);
// bool Client::checkPingTimeouts(int timeoutMs);

// ConnectionManager.cpp
// void ConnectionManager::sendPingToClient(Client &client);
// void ConnectionManager::sendPingToAllClients();
// void ConnectionManager::checkAllPingTimeouts(int timeoutMs);
