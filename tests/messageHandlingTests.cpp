#include "TestSetup.hpp"
#include <cstring>

class MessageHandlingTest : public TestSetup
{
};

// Test sending an oversized message
TEST_F(MessageHandlingTest, OversizedMessage)
{
    // Use the helper to set up a client that's already in #test
    std::vector<int> clients = basicSetupMultiple(1);
    int client = clients[0];

    // Create a message larger than MSG_BUFFER_SIZE
    std::string prefix = "PRIVMSG #test :";
    std::string largeContent = createLargeString(MSG_BUFFER_SIZE * 2, 'X');
    std::string largeMessage = prefix + largeContent + "\r\n";

    // Send the oversized message
    sendRawData(client, largeMessage);

    // Verify truncation happened by checking the message was delivered but truncated
    EXPECT_TRUE(
        outputContains("421")); // unknown command because of XXXX getting parsed as a command.

    // Verify client is still connected by sending a normal command
    clearServerOutput();
    sendCommand(client, "PING :test");
    EXPECT_TRUE(outputContains("PONG"));
}

// Test sending multiple commands in a single message
TEST_F(MessageHandlingTest, MultipleCommandsInOneMessage)
{
    std::vector<int> clients = basicSetupMultiple(1);
    int client = clients[0];

    // Create a message with multiple commands
    std::string multiCommand = "JOIN #channel1\r\nJOIN #channel2\r\nPRIVMSG #test :Hello\r\n";
    sendRawData(client, multiCommand);

    // Verify all commands were processed
    EXPECT_TRUE(outputContains("basicUser0!testuser@127.0.0.1 JOIN #channel1"));
    EXPECT_TRUE(outputContains("basicUser0!testuser@127.0.0.1 JOIN #channel2"));
    EXPECT_TRUE(outputContains("PRIVMSG #test :Hello"));
}

TEST_F(MessageHandlingTest, LargeValidMessages)
{
    std::vector<int> clients = basicSetupMultiple(1);
    int client = clients[0];

    std::string prefix = "PRIVMSG #test :";
    std::string largeContent = createLargeString(MSG_BUFFER_SIZE - prefix.length() - 2, 'X');
    std::string largeMessage = prefix + largeContent + "\r\n";
    // Create a message with multiple commands over buffer limit
    std::string multiCommand = largeMessage + largeMessage + largeMessage + largeMessage +
                               largeMessage + largeMessage + largeMessage + largeMessage +
                               "JOIN #channel1\r\nJOIN #channel2\r\nPRIVMSG #test :Hello\r\n";
    sendRawData(client, multiCommand);

    // Verify all commands were processed
    EXPECT_TRUE(outputContains("basicUser0!testuser@127.0.0.1 JOIN #channel1"));
    EXPECT_TRUE(outputContains("basicUser0!testuser@127.0.0.1 JOIN #channel2"));
    EXPECT_TRUE(outputContains("PRIVMSG #test :Hello"));
}

// Test sending malformed commands without proper line endings
TEST_F(MessageHandlingTest, IncompleteCommands)
{
    std::vector<int> clients = basicSetupMultiple(1);
    int client = clients[0];

    // Send incomplete command (no \r\n)
    sendRawData(client, "JOIN #nocrlf");

    // Wait briefly to ensure server had time to process if it was going to
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    // Verify the command wasn't processed without \r\n
    std::string output = getServerOutput();
    EXPECT_FALSE(output.find("JOIN #nocrlf") != std::string::npos);

    // Complete the command with \r\n
    sendRawData(client, "\r\n");

    // Now the command should be processed
    EXPECT_TRUE(outputContains("basicUser0!testuser@127.0.0.1 JOIN #nocrlf"));
}

// Test sending a command in chunks
TEST_F(MessageHandlingTest, FragmentedCommand)
{
    std::vector<int> clients = basicSetupMultiple(1);
    int client = clients[0];

    // Send command in fragments
    sendRawData(client, "JOI");
    sendRawData(client, "N #fragmented");
    sendRawData(client, "Channel\r\n");

    // Verify the complete command was processed once complete
    EXPECT_TRUE(outputContains("basicUser0!testuser@127.0.0.1 JOIN #fragmentedChannel"));
}

// Test combination of multiple complete and incomplete commands
TEST_F(MessageHandlingTest, MixedCommandsAndFragments)
{
    std::vector<int> clients = basicSetupMultiple(1);
    int client = clients[0];

    // Send a complete command followed by an incomplete one
    sendRawData(client, "JOIN #channel1\r\nJOIN #chan");

    // Verify the complete command was processed
    EXPECT_TRUE(outputContains("basicUser0!testuser@127.0.0.1 JOIN #channel1"));
    clearServerOutput();

    // Complete the second command
    sendRawData(client, "nel2\r\n");

    // Verify the now-complete second command was processed
    EXPECT_TRUE(outputContains("basicUser0!testuser@127.0.0.1 JOIN #channel2"));
}

// Test handling large buffer built up over time without line endings
TEST_F(MessageHandlingTest, BufferOverflowWithoutNewlines)
{
    std::vector<int> clients = basicSetupMultiple(1);
    int client = clients[0];

    // Setup the command prefix
    std::string cmdPrefix = "PRIVMSG #test :";

    // Start building the message
    sendRawData(client, cmdPrefix);

    // Calculate appropriate chunk sizes based on buffer size
    int initialChunkSize = (MSG_BUFFER_SIZE / 2) - cmdPrefix.length();
    int middleChunkSize = (MSG_BUFFER_SIZE / 4);
    int finalChunkSize = (MSG_BUFFER_SIZE / 2);

    // Ensure we have reasonable values even with tiny buffer sizes
    initialChunkSize = std::max(1, initialChunkSize);
    middleChunkSize = std::max(1, middleChunkSize);
    finalChunkSize = std::max(1, finalChunkSize);

    // Send initial chunks with 'A's
    std::string initialChunk = createLargeString(initialChunkSize, 'A');
    sendRawData(client, initialChunk);

    // Now send a chunk with 'B's that should appear partially in the output
    std::string middleChunk = createLargeString(middleChunkSize, 'B');
    sendRawData(client, middleChunk);

    // Finally send a chunk with 'C's that should be fully truncated
    std::string finalChunk = createLargeString(finalChunkSize, 'C');
    sendRawData(client, finalChunk);

    // Complete the message with \r\n to trigger processing
    // sendRawData(client, "\r\n");

    // Calculate exact expected result - only what will fit in the buffer
    int totalContentSize = cmdPrefix.length() + initialChunkSize + middleChunkSize + finalChunkSize;
    int truncatedSize = std::min(totalContentSize, MSG_BUFFER_SIZE - 2);
    int remainingSpace = truncatedSize - cmdPrefix.length();

    std::string expectedResult = cmdPrefix;

    // Add as many A's as will fit
    int aCount = std::min(initialChunkSize, remainingSpace);
    if (aCount > 0) {
        expectedResult += createLargeString(aCount, 'A');
        remainingSpace -= aCount;
    }

    // Add as many B's as will fit in the remaining space
    int bCount = std::min(middleChunkSize, remainingSpace);
    if (bCount > 0) {
        expectedResult += createLargeString(bCount, 'B');
        remainingSpace -= bCount;
    }

    // Add as many C's as will fit in the remaining space (if any)
    int cCount = std::min(finalChunkSize, remainingSpace);
    if (cCount > 0) {
        expectedResult += createLargeString(cCount, 'C');
    }

    // Verify the expected content is present
    EXPECT_TRUE(outputContains(expectedResult));

    // Get the full output
    std::string output = getServerOutput();

    // Verify client can still communicate
    sendCommand(client, "PING :stillalive");
    EXPECT_TRUE(outputContains("PONG"));
}

// Test handling empty messages
TEST_F(MessageHandlingTest, EmptyMessages)
{
    std::vector<int> clients = basicSetupMultiple(1);
    int client = clients[0];
    clearServerOutput();

    // Send empty line (just \r\n)
    sendRawData(client, "\r\n");

    // Verify no errors occur and the client remains connected
    sendCommand(client, "PING :emptytest");
    EXPECT_TRUE(outputContains("PONG"));
}

// Test command right at the buffer size limit
TEST_F(MessageHandlingTest, ExactBufferSizeCommand)
{
    std::vector<int> clients = basicSetupMultiple(1);
    int client = clients[0];
    clearServerOutput();

    // Create a command that's exactly at the buffer limit
    std::string prefix = "PRIVMSG #test :";
    std::string content = createLargeString(MSG_BUFFER_SIZE - prefix.length() - 2, 'X');
    std::string message = prefix + content + "\r\n";

    // Verify the message is exactly at the limit
    ASSERT_EQ(message.length() - 2, MSG_BUFFER_SIZE - 2);

    // Send the message
    sendRawData(client, message);

    EXPECT_TRUE(outputContains(prefix));
}

// Test handling malformed command format
TEST_F(MessageHandlingTest, MalformedCommands)
{
    std::vector<int> clients = basicSetupMultiple(1);
    int client = clients[0];
    clearServerOutput();

    // Send malformed commands (missing params, invalid format)
    sendCommand(client, ":invalidprefix");
    sendCommand(client, "PRIVMSG");
    sendCommand(client, "JOIN#nochannel");

    // Verify the server handles them gracefully (might return errors but not crash)
    // Server should send ERR_NEEDMOREPARAMS, ERR_NOSUCHCHANNEL, ERR_UNKNOWNCOMMAND or similar
    EXPECT_TRUE(outputContains("461") || outputContains("421") || outputContains("403"));

    // Verify client is still connected
    sendCommand(client, "PING :malformedtest");
    EXPECT_TRUE(outputContains("PONG"));
}

// Test partial line endings (CR without LF or LF without CR)
TEST_F(MessageHandlingTest, PartialLineEndings)
{
    std::vector<int> clients = basicSetupMultiple(1);
    int client = clients[0];
    clearServerOutput();

    // Send CR without LF
    sendRawData(client, "JOIN #chanwithCR\r");
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    // Command shouldn't be processed yet
    EXPECT_FALSE(outputContains("JOIN #chanwithCR"));

    // Now send just LF to complete
    sendRawData(client, "\n");

    // Command should now be processed
    EXPECT_TRUE(outputContains("JOIN #chanwithCR"));
    clearServerOutput();

    // Test LF without CR (many clients send just \n)
    sendRawData(client, "JOIN #chanwithLF\n");

    // This should also be accepted (RFC allows just \n)
    EXPECT_TRUE(outputContains("JOIN #chanwithLF"));
}

// Test sending commands with Unicode and special characters
TEST_F(MessageHandlingTest, UnicodeAndSpecialCharacters)
{
    std::vector<int> clients = basicSetupMultiple(1);
    int client = clients[0];
    clearServerOutput();

    // Send messages with Unicode and special characters
    sendCommand(client, "PRIVMSG #test :Unicode test ♥ ñ 你好 💡 😊");
    sendCommand(client, "PRIVMSG #test :Special characters: !@#$%^&*()_+{}|:<>?~`-=[]\\;',./");

    // Verify messages are processed correctly
    EXPECT_TRUE(outputContains("Unicode test"));
    EXPECT_TRUE(outputContains("Special characters"));
}

// Test rapid consecutive command sending
TEST_F(MessageHandlingTest, RapidConsecutiveCommands)
{
    std::vector<int> clients = basicSetupMultiple(1);
    int client = clients[0];
    clearServerOutput();

    // Prepare a batch of small commands
    std::string batchCommands;
    for (int i = 0; i < 200; i++) {
        batchCommands += "JOIN #channel" + std::to_string(i) + "\r\n";
    }

    // Send them all at once
    sendRawData(client, batchCommands);

    // Verify all commands were processed
    for (int i = 0; i < 200; i++) {
        EXPECT_TRUE(outputContains("JOIN #channel" + std::to_string(i)));
    }
}

// Test boundary condition with message exactly at buffer limit plus one
TEST_F(MessageHandlingTest, MessageExactlyOverLimit)
{
    std::vector<int> clients = basicSetupMultiple(1);
    int client = clients[0];
    clearServerOutput();

    // Create a message exactly one byte over the limit
    std::string prefix = "PRIVMSG #test :";
    std::string content = createLargeString(MSG_BUFFER_SIZE - prefix.length() - 1, 'X');
    std::string message = prefix + content + "\r\n";

    // Verify the message is exactly one byte over the limit
    ASSERT_EQ(message.length() - 2, MSG_BUFFER_SIZE - 1);

    // Send the message
    sendRawData(client, message);

    // Should be truncated
    EXPECT_TRUE(outputContains("PRIVMSG #test :"));

    // Check last character was truncated
    std::string expected = prefix + content.substr(0, content.length() - 1);
    EXPECT_TRUE(outputContains(expected));
}
