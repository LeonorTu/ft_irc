#include "TestSetup.hpp"

class InviteTests : public TestSetup
{
protected:
    InviteTests()
        : TestSetup(true)
    {}
};

// Test basic invitation functionality
TEST_F(InviteTests, BasicInvite)
{
    std::vector<int> clients = basicSetupMultiple(2);
    int client0 = clients[0]; // op
    int client1 = clients[1]; // regular user

    // Create a new channel for testing
    sendCommand(client0, "JOIN #invitetest");
    clearServerOutput();

    // Have client1 leave the #test channel
    sendCommand(client1, "PART #test");
    clearServerOutput();

    // Client0 invites client1 to the channel
    sendCommand(client0, "INVITE basicUser1 #invitetest");

    // Check for proper invite notification
    EXPECT_TRUE(outputContains("341 basicUser0 basicUser1 #invitetest")); // RPL_INVITING to user0
    EXPECT_TRUE(outputContains(
        ":basicUser0!testuser@127.0.0.1 INVITE basicUser1 #invitetest")); // INVITE to user1
    clearServerOutput();

    // Client1 joins the channel using the invitation
    sendCommand(client1, "JOIN #invitetest");
    EXPECT_TRUE(outputContains(":basicUser1!testuser@127.0.0.1 JOIN #invitetest"));
    clearServerOutput();
}

// Test inviting a user who is already in the channel
TEST_F(InviteTests, AlreadyInChannel)
{
    std::vector<int> clients = basicSetupMultiple(2);
    int client0 = clients[0]; // op
    int client1 = clients[1]; // already in the channel from setup

    // Mark client1 as used to avoid unused variable warning
    (void)client1;

    // Try to invite client1 to #test (already in channel)
    sendCommand(client0, "INVITE basicUser1 #test");
    EXPECT_TRUE(outputContains("443 basicUser0 basicUser1 #test :is already on channel"));
    clearServerOutput();
}

// Test inviting a non-existent user
TEST_F(InviteTests, NonExistentUser)
{
    std::vector<int> clients = basicSetupMultiple(1);
    int client0 = clients[0];

    // Try to invite non-existent user
    sendCommand(client0, "INVITE nonexistentuser #test");
    EXPECT_TRUE(outputContains("401 basicUser0 nonexistentuser :No such nick"));
    clearServerOutput();
}

// Test invite-only channel
TEST_F(InviteTests, InviteOnlyChannel)
{
    std::vector<int> clients = basicSetupMultiple(2);
    int client0 = clients[0]; // op
    int client1 = clients[1]; // regular user

    // Create a new channel and set it to invite-only
    sendCommand(client0, "JOIN #inviteonly");
    sendCommand(client0, "MODE #inviteonly +i");
    clearServerOutput();

    // Have client1 try to join without invitation
    sendCommand(client1, "JOIN #inviteonly");
    EXPECT_TRUE(outputContains("473 basicUser1 #inviteonly :Cannot join channel (+i)"));
    clearServerOutput();

    // Now invite client1
    sendCommand(client0, "INVITE basicUser1 #inviteonly");
    clearServerOutput();

    // Client1 should now be able to join
    sendCommand(client1, "JOIN #inviteonly");
    EXPECT_TRUE(outputContains(":basicUser1!testuser@127.0.0.1 JOIN #inviteonly"));
    clearServerOutput();
}

// Test invitation from non-operator in invite-only channel
TEST_F(InviteTests, NonOperatorInvite)
{
    std::vector<int> clients = basicSetupMultiple(3);
    int client0 = clients[0]; // op
    int client1 = clients[1]; // regular user
    int client2 = clients[2]; // user to be invited

    // Create a new channel and set it to invite-only
    sendCommand(client0, "JOIN #nonopchannel");
    sendCommand(client1, "JOIN #nonopchannel");
    sendCommand(client0, "MODE #nonopchannel +i");
    clearServerOutput();

    // Have client2 leave the #test channel
    sendCommand(client2, "PART #test");
    clearServerOutput();

    // Non-operator client1 tries to invite client2, doesnt work on inviteonly mode
    sendCommand(client1, "INVITE basicUser2 #nonopchannel");
    EXPECT_TRUE(outputContains("482 basicUser1 #nonopchannel :You're not channel operator"));
    clearServerOutput();

    // Client2 tries to join
    sendCommand(client2, "JOIN #nonopchannel");
    EXPECT_TRUE(outputContains("473")); // invite only mode error
    clearServerOutput();

    // operator sends the invite
    sendCommand(client0, "INVITE basicUser2 #nonopchannel");
    EXPECT_TRUE(outputContains("341")); // RPL_INVITING
    EXPECT_TRUE(outputContains(
        ":basicUser0!testuser@127.0.0.1 INVITE basicUser2 #nonopchannel")); // invite is sent out

    sendCommand(client2, "JOIN #nonopchannel");
    EXPECT_TRUE(outputContains(":basicUser2!testuser@127.0.0.1 JOIN #nonopchannel"));
}

// Test inviting to a non-existent channel
TEST_F(InviteTests, NonExistentChannel)
{
    std::vector<int> clients = basicSetupMultiple(2);
    int client0 = clients[0];

    // Try to invite to non-existent channel
    sendCommand(client0, "INVITE basicUser1 #nonexistent");
    EXPECT_TRUE(outputContains("403 basicUser0 #nonexistent :No such channel"));
    clearServerOutput();
}

// Test user not in channel tries to invite
TEST_F(InviteTests, InviterNotInChannel)
{
    std::vector<int> clients = basicSetupMultiple(3);
    int client0 = clients[0];
    int client1 = clients[1];
    int client2 = clients[2];
    (void)client2;

    sendCommand(client0, "JOIN #newchannel");
    clearServerOutput();

    // User not in channel tries to invite
    sendCommand(client1, "INVITE basicUser2 #newchannel");
    EXPECT_TRUE(outputContains("442 basicUser1 #newchannel :You're not on that channel"));
    clearServerOutput();
}
