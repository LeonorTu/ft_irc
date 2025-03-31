#include "GeminiBot.hpp"
#include <iostream>
#include <csignal>
#include <cstdlib>
#include <thread>

static bool running = true;

void signalHandler(int signum)
{
    std::cout << "\nSignal (" << signum << ") received. Shutting down...\n";
    running = false;
}

void printUsage(const char *programName)
{
    std::cerr << "Usage: " << programName << " <server> <port> <password> <nickname> <api_key>\n"
              << "Example: " << programName
              << " irc.example.com 6667 serverpass botname YOUR_GEMINI_API_KEY\n";
}

int main(int argc, char *argv[])
{
    if (argc != 6) {
        printUsage(argv[0]);
        return 1;
    }

    // Register signal handler for graceful shutdown
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);

    try {
        // Parse command line arguments
        std::string server = argv[1];
        int port = std::atoi(argv[2]);
        std::string password = argv[3];
        std::string nickname = argv[4];
        std::string api_key = argv[5];

        // Create and configure bot
        GeminiBot bot(server, port, password, nickname, api_key);

        // Connect to IRC server
        if (!bot.connect()) {
            std::cerr << "Failed to connect to IRC server\n";
            return 1;
        }

        std::cout << "Bot connected successfully. Press Ctrl+C to quit.\n";

        // Main loop
        while (running) {
            if (!bot.isConnected()) {
                std::cerr << "Lost connection to server. Attempting to reconnect...\n";
                if (!bot.connect()) {
                    std::cerr << "Reconnection failed. Retrying in 5 seconds...\n";
                    std::this_thread::sleep_for(std::chrono::seconds(5));
                    continue;
                }
            }

            // Process messages
            bot.processServerMessages();

            // Small sleep to prevent CPU hogging
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }

        std::cout << "Shutting down bot...\n";
    }
    catch (const std::exception &e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
