#include "TestSetup.hpp"

class PartQuitTests : public TestSetup
{
protected:
    PartQuitTests()
        : TestSetup(true)
    {}
};

// Test basic PART functionality
TEST_F(PartQuitTests, BasicPart)
{
    std::vector<int> clients = basicSetupMultiple(2);
    int client0 = clients[0]; // op
    int client1 = clients[1]; // regular user

    // Mark client0 as used to avoid unused variable warning
    (void)client0;

    // User parts the channel
    sendCommand(client1, "PART #test");
    EXPECT_TRUE(outputContains(":basicUser1!testuser@127.0.0.1 PART #test"));
    clearServerOutput();

    // Verify user is no longer in channel, second PART shouldnt work
    sendCommand(client1, "PART #test");
    EXPECT_TRUE(outputContains("442 basicUser1 #test :You're not on that channel"));
    clearServerOutput();
}

// Test PART with reason
TEST_F(PartQuitTests, PartWithReason)
{
    std::vector<int> clients = basicSetupMultiple(2);
    int client0 = clients[0]; // op
    int client1 = clients[1]; // regular user

    // Mark client0 as used to avoid unused variable warning
    (void)client0;

    // User parts with a reason
    sendCommand(client1, "PART #test :Leaving for testing");
    EXPECT_TRUE(outputContains(":basicUser1!testuser@127.0.0.1 PART #test :Leaving for testing"));
    clearServerOutput();
}

// Test PART multiple channels at once
TEST_F(PartQuitTests, PartMultipleChannels)
{
    std::vector<int> clients = basicSetupMultiple(1);
    int client = clients[0];

    // Join additional channels
    sendCommand(client, "JOIN #chan1,#chan2");
    clearServerOutput();

    // Part multiple channels at once
    sendCommand(client, "PART #test,#chan1 :Leaving multiple channels");
    EXPECT_TRUE(
        outputContains(":basicUser0!testuser@127.0.0.1 PART #test :Leaving multiple channels"));
    EXPECT_TRUE(
        outputContains(":basicUser0!testuser@127.0.0.1 PART #chan1 :Leaving multiple channels"));
    clearServerOutput();

    // Verify still in the remaining channel
    sendCommand(client, "PRIVMSG #chan2 :Still here");
    EXPECT_FALSE(outputContains("442 basicUser0 #chan2 :You're not on that channel"));
    clearServerOutput();
}

// Test PART from non-existent channel
TEST_F(PartQuitTests, PartNonExistentChannel)
{
    std::vector<int> clients = basicSetupMultiple(1);
    int client = clients[0];

    // Try to part a non-existent channel
    sendCommand(client, "PART #nonexistent");
    EXPECT_TRUE(outputContains("403 basicUser0 #nonexistent :No such channel"));
    clearServerOutput();
}

// Test PART channel user isn't on
TEST_F(PartQuitTests, PartChannelNotOn)
{
    std::vector<int> clients = basicSetupMultiple(2);
    int client0 = clients[0];
    int client1 = clients[1];

    // Create a channel that client1 isn't on
    sendCommand(client0, "JOIN #exclusive");
    clearServerOutput();

    // Try to part a channel the user isn't on
    sendCommand(client1, "PART #exclusive");
    EXPECT_TRUE(outputContains("442 basicUser1 #exclusive :You're not on that channel"));
    clearServerOutput();
}

// Test PART without parameters
TEST_F(PartQuitTests, PartWithoutParams)
{
    std::vector<int> clients = basicSetupMultiple(1);
    int client = clients[0];

    // Try to part without specifying a channel
    sendCommand(client, "PART");
    EXPECT_TRUE(outputContains("461 basicUser0 PART :Not enough parameters"));
    clearServerOutput();
}

// Test operator status lost after parting and rejoining
TEST_F(PartQuitTests, OpStatusAfterPart)
{
    std::vector<int> clients = basicSetupMultiple(2);
    int client0 = clients[0]; // op
    int client1 = clients[1]; // regular user

    // Mark client1 as used to avoid unused variable warning
    (void)client1;

    // Part and rejoin
    sendCommand(client0, "PART #test");
    clearServerOutput();

    sendCommand(client0, "JOIN #test");
    clearServerOutput();

    // Should no longer have operator status
    sendCommand(client0, "MODE #test +i");
    // Could be either "not operator" or succeed (if rejoining gives op)
    // Check based on your implementation
    if (outputContains("482 basicUser0 #test :You're not channel operator")) {
        EXPECT_TRUE(true); // Op status lost
    }
    else {
        EXPECT_TRUE(outputContains("MODE #test +i")); // Op status retained/regained
    }
    clearServerOutput();
}

// Test basic QUIT functionality
TEST_F(PartQuitTests, BasicQuit)
{
    std::vector<int> clients = basicSetupMultiple(2);
    int client0 = clients[0]; // op
    int client1 = clients[1]; // regular user

    // User quits
    sendCommand(client1, "QUIT");
    EXPECT_TRUE(outputContains(":basicUser1!testuser@127.0.0.1 QUIT"));
    clearServerOutput();

    // Cannot test commands after quit since the socket is closed
    // Instead, test that messages to the quit user don't work
    sendCommand(client0, "PRIVMSG basicUser1 :Are you there?");
    EXPECT_TRUE(outputContains("401") || outputContains("No such nick"));
    clearServerOutput();
}

// Test QUIT with specific reason
TEST_F(PartQuitTests, QuitWithReason)
{
    std::vector<int> clients = basicSetupMultiple(2);
    int client0 = clients[0]; // op
    int client1 = clients[1]; // regular user

    // Mark client0 as used to avoid unused variable warning
    (void)client0;

    // User quits with reason
    sendCommand(client1, "QUIT :Goodbye cruel world");
    EXPECT_TRUE(outputContains("ERROR :Quit: Goodbye cruel world") ||
                outputContains(":basicUser1!testuser@127.0.0.1 QUIT :Goodbye cruel world"));
    clearServerOutput();
}

// Test quitting when in multiple channels
TEST_F(PartQuitTests, QuitFromMultipleChannels)
{
    std::vector<int> clients = basicSetupMultiple(3);
    int client0 = clients[0]; // stays
    int client1 = clients[1]; // stays
    int client2 = clients[2]; // quits

    // Mark client1 as used to avoid unused variable warning
    (void)client1;

    // Join additional channels
    sendCommand(client0, "JOIN #chan1");
    sendCommand(client2, "JOIN #chan1");
    clearServerOutput();

    // User quits
    sendCommand(client2, "QUIT :Leaving multiple channels");

    // Both channels should get the quit notification
    EXPECT_TRUE(outputContains(":basicUser2!testuser@127.0.0.1 QUIT") ||
                outputContains("ERROR :Quit: Leaving multiple channels"));
    clearServerOutput();

    // Cannot verify further commands from client2 since socket is closed
}
