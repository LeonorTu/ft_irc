#include <string>
#include <unordered_map>
#include <functional>

class CommandProcessor
{
public:
    // Constructor takes client (maybe change to just nickname of the sender?) and raw command string
    CommandProcessor(std::string &sender, const std::string &rawString);
    ~CommandProcessor();

    // Command parameters struct - contains all data needed by handlers
    struct CommandContext
    {
        // Command identification
        std::string command;
        std::string source;
        std::string sender; // Not sure if we need that separately? might be needed for some commands

        // User identifiers
        std::string clientNickname; // For join, part, quit, mode commands
        std::string oldNickname;    // For NICK command
        std::string newNickname;    // For NICK command
        std::string targetNickname; // For KICK and INVITE commands

        // Channel parameters
        std::string channelName; // Channel being operated on
        std::string channelKey;  // For JOIN with password
        std::string newTopic;    // For TOPIC command

        // Mode operations
        std::string mode;       // Channel mode changes ("+o", "-i", etc.)
        std::string parameters; // Parameters for mode commands

        // Messages and reasons
        std::string reason;      // For QUIT and PART
        std::string kickComment; // Reason for KICK
    };

    // Execute the parsed and validated command, this will just choose the appropriate function from the map, so cool.
    void executeCommand();

private:
    // Parsed command context
    CommandContext _context;

    std::unordered_map<std::string, std::function<void(const CommandContext &)>> _commandHandlers;

    // Private methods
    void parseCommand(const std::string &rawString);
    void setupCommandHandlers();
};