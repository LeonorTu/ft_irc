#include "TestSetup.hpp"

class JoinTests : public TestSetup
{
protected:
    JoinTests()
        : TestSetup(true)
    {}
};

// Test basic join functionality
TEST_F(JoinTests, BasicJoin)
{
    std::vector<int> clients = basicSetupMultiple(2);
    // The users have already joined #test in the setup
    // Just verify they're there by sending a command in the channel

    sendCommand(clients[0], "PRIVMSG #test :Hello");
    EXPECT_TRUE(outputContains(":basicUser0!testuser@127.0.0.1 PRIVMSG #test :Hello"));
    clearServerOutput();
}

// Test joining multiple channels
TEST_F(JoinTests, MultipleChannels)
{
    std::vector<int> clients = basicSetupMultiple(1);
    int client = clients[0];

    // Join another channel
    sendCommand(client, "JOIN #second,#third");
    EXPECT_TRUE(outputContains(":basicUser0!testuser@127.0.0.1 JOIN #second"));
    EXPECT_TRUE(outputContains(":basicUser0!testuser@127.0.0.1 JOIN #third"));
    clearServerOutput();
}

// Test channel with key
TEST_F(JoinTests, ChannelWithKey)
{
    std::vector<int> clients = basicSetupMultiple(2);
    int client1 = clients[0];
    int client2 = clients[1];

    // Create a new channel with key
    sendCommand(client1, "JOIN #keychannel");
    sendCommand(client1, "MODE #keychannel +k secretkey");
    clearServerOutput();

    // Part both users from the #test channel
    sendCommand(client1, "PART #test");
    sendCommand(client2, "PART #test");
    clearServerOutput();

    // Wrong key attempt
    sendCommand(client2, "JOIN #keychannel wrongkey");
    EXPECT_TRUE(outputContains("475 basicUser1 #keychannel :Cannot join channel (+k)"));
    clearServerOutput();

    // Correct key attempt
    sendCommand(client2, "JOIN #keychannel secretkey");
    EXPECT_TRUE(outputContains(":basicUser1!testuser@127.0.0.1 JOIN #keychannel"));
    clearServerOutput();
}

// Test invite-only channel
TEST_F(JoinTests, InviteOnlyChannel)
{
    std::vector<int> clients = basicSetupMultiple(3);
    int client1 = clients[0];
    int client2 = clients[1];
    int client3 = clients[2];

    // Create a new channel and set it to invite-only
    sendCommand(client1, "JOIN #invite");
    sendCommand(client1, "MODE #invite +i");
    clearServerOutput();

    // Try to join without invite
    sendCommand(client2, "JOIN #invite");
    EXPECT_TRUE(outputContains("473 basicUser1 #invite :Cannot join channel (+i)"));
    clearServerOutput();

    // Send an invite and join
    sendCommand(client1, "INVITE basicUser1 #invite");
    sendCommand(client2, "JOIN #invite");
    EXPECT_TRUE(outputContains(":basicUser1!testuser@127.0.0.1 JOIN #invite"));
    clearServerOutput();

    // Test invite and key together
    sendCommand(client1, "MODE #invite +k testkey");
    sendCommand(client1, "INVITE basicUser2 #invite");
    sendCommand(client3, "JOIN #invite"); // Missing key
    EXPECT_TRUE(outputContains("475 basicUser2 #invite :Cannot join channel (+k)"));
    clearServerOutput();

    sendCommand(client3, "JOIN #invite testkey");
    EXPECT_TRUE(outputContains(":basicUser2!testuser@127.0.0.1 JOIN #invite"));
    clearServerOutput();
}

// Test user limit
TEST_F(JoinTests, UserLimit)
{
    std::vector<int> clients = basicSetupMultiple(3);
    int client1 = clients[0];

    // Set user limit to 1
    sendCommand(client1, "MODE #test +l 1");
    EXPECT_TRUE(outputContains(":basicUser0!testuser@127.0.0.1 MODE #test +l 1"));
    clearServerOutput();

    // Create a new client and try to join (should fail due to limit)
    int newClient = connectClient();
    ASSERT_GT(newClient, 0);
    registerClient(newClient, "limituser");

    sendCommand(newClient, "JOIN #test");
    EXPECT_TRUE(outputContains("471 limituser #test :Cannot join channel (+l)"));
    clearServerOutput();
}

// Test JOIN 0 (leaving all channels)
TEST_F(JoinTests, JoinZero)
{
    std::vector<int> clients = basicSetupMultiple(1);
    int client = clients[0];

    // Join multiple channels
    sendCommand(client, "JOIN #chan1,#chan2");
    clearServerOutput();

    // Use JOIN 0 to leave all channels
    sendCommand(client, "JOIN 0");
    EXPECT_TRUE(outputContains(":basicUser0!testuser@127.0.0.1 PART #test :"));
    EXPECT_TRUE(outputContains(":basicUser0!testuser@127.0.0.1 PART #chan1 :"));
    EXPECT_TRUE(outputContains(":basicUser0!testuser@127.0.0.1 PART #chan2 :"));
    clearServerOutput();
}
