#include <CommandProcessor.hpp>
#include <commandHandlers.hpp>

void CommandProcessor::executeCommand()
{
    // Check if the command exists in our handlers map
    auto it = _commandHandlers.find(_command);

    if (it != _commandHandlers.end()) {
        // Call the function with the command context
        it->second(_context);
    }
    else {
        std::cerr << "ignoring command: " << _command << std::endl;
    }
}

void CommandProcessor::setupCommandHandlers()
{
    _commandHandlers["NICK"] = nick;
}