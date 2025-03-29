#include "LogBot.hpp"
#include <iostream>
#include <string>

void printUsage(const char* programName) {
    std::cout << "Usage: " << programName << " [server] [port] [password] [nickname] [channel]" << std::endl;
    std::cout << std::endl;
    std::cout << "Parameters:" << std::endl;
    std::cout << "  server    - IRC server address (default: 127.0.0.1)" << std::endl;
    std::cout << "  port      - IRC server port (default: 6667)" << std::endl;
    std::cout << "  password  - Server password (default: password)" << std::endl;
    std::cout << "  nickname  - Bot nickname (default: LogBot)" << std::endl;
    std::cout << "  channel   - Channel to join (default: #logs)" << std::endl;
    std::cout << std::endl;
    std::cout << "Example: " << programName << " irc.example.com 6667 mypassword MyLogBot #channel" << std::endl;
    std::cout << std::endl;
    std::cout << "The bot reads from stdin and sends each line to the specified IRC channel." << std::endl;
}

int main(int argc, char* argv[]) {
    // Print usage if --help is provided
    if (argc > 1 && (std::string(argv[1]) == "--help" || std::string(argv[1]) == "-h")) {
        printUsage(argv[0]);
        return 0;
    }
    
    // Default settings
    std::string server = "127.0.0.1";
    int port = 6667;
    std::string password = "password";
    std::string nickname = "LogBot";
    std::string channel = "#logs";
    
    // Allow command line parameter overrides
    if (argc > 1) server = argv[1];
    if (argc > 2) port = std::stoi(argv[2]);
    if (argc > 3) password = argv[3];
    if (argc > 4) nickname = argv[4];
    if (argc > 5) channel = argv[5];
    
    // Create and connect the log bot
    LogBot bot(server, port, password, nickname, channel);
    
    if (!bot.connect()) {
        std::cerr << "Failed to connect to IRC server. Exiting." << std::endl;
        return 1;
    }
    
    std::cerr << "Connected to IRC server. Sending log messages to " << channel << std::endl;
    std::cerr << "Reading from stdin, press Ctrl+D to exit." << std::endl;
    
    // Read from stdin and send to IRC
    std::string line;
    while (std::getline(std::cin, line)) {
        if (!line.empty()) {
            bot.logMessage(line);
        }
        
        // Process any server messages (like PING)
        bot.processServerMessages();
    }
    
    std::cerr << "End of input. Shutting down." << std::endl;
    return 0;
}
