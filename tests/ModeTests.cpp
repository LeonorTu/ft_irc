#include "TestSetup.hpp"

class ModeTests : public TestSetup
{
protected:
    ModeTests()
        : TestSetup(true)
    {}
};

// Test basic channel mode viewing and setting
TEST_F(ModeTests, BasicModes)
{
    std::vector<int> clients = basicSetupMultiple(2);
    int client0 = clients[0]; // op
    int client1 = clients[1]; // regular user

    // Mark unused variable as used
    (void)client1;

    // Test viewing channel modes (should show default modes)
    sendCommand(client0, "MODE #test");
    EXPECT_TRUE(outputContains("324 basicUser0 #test"));
    clearServerOutput();

    // Test setting invite-only mode
    sendCommand(client0, "MODE #test +i");
    EXPECT_TRUE(outputContains(":basicUser0!testuser@127.0.0.1 MODE #test +i"));
    clearServerOutput();

    // Verify mode is set by viewing modes
    sendCommand(client0, "MODE #test");
    EXPECT_TRUE(outputContains("324 basicUser0 #test +i"));
    clearServerOutput();

    // Test removing mode
    sendCommand(client0, "MODE #test -i");
    EXPECT_TRUE(outputContains(":basicUser0!testuser@127.0.0.1 MODE #test -i"));
    clearServerOutput();
}

// Test channel key mode
TEST_F(ModeTests, ChannelKey)
{
    std::vector<int> clients = basicSetupMultiple(2);
    int client0 = clients[0]; // op
    int client1 = clients[1]; // regular user

    // Mark unused variable as used
    (void)client1;

    // Set channel key
    sendCommand(client0, "MODE #test +k testkey");
    EXPECT_TRUE(outputContains(":basicUser0!testuser@127.0.0.1 MODE #test +k testkey"));
    clearServerOutput();

    // Create a new client to test joining with key
    int client2 = connectClient();
    ASSERT_GT(client2, 0);
    registerClient(client2, "keyuser");

    // Try joining without key
    sendCommand(client2, "JOIN #test");
    EXPECT_TRUE(outputContains("475 keyuser #test :Cannot join channel (+k)"));
    clearServerOutput();

    // Try joining with correct key
    sendCommand(client2, "JOIN #test testkey");
    EXPECT_TRUE(outputContains(":keyuser!testuser@127.0.0.1 JOIN #test"));
    clearServerOutput();

    // Remove key
    sendCommand(client0, "MODE #test -k");
    EXPECT_TRUE(outputContains(":basicUser0!testuser@127.0.0.1 MODE #test -k"));
    clearServerOutput();
}

// Test user limit mode
TEST_F(ModeTests, UserLimit)
{
    std::vector<int> clients = basicSetupMultiple(2);
    int client0 = clients[0]; // op

    // Set user limit to 2 (current number of users)
    sendCommand(client0, "MODE #test +l 2");
    EXPECT_TRUE(outputContains(":basicUser0!testuser@127.0.0.1 MODE #test +l 2"));
    clearServerOutput();

    // Create a new client and try to join
    int client2 = connectClient();
    ASSERT_GT(client2, 0);
    registerClient(client2, "limituser");

    // Try joining (should fail due to limit)
    sendCommand(client2, "JOIN #test");
    EXPECT_TRUE(outputContains("471 limituser #test :Cannot join channel (+l)"));
    clearServerOutput();

    // Increase limit and try again
    sendCommand(client0, "MODE #test +l 3");
    EXPECT_TRUE(outputContains(":basicUser0!testuser@127.0.0.1 MODE #test +l 3"));
    clearServerOutput();

    sendCommand(client2, "JOIN #test");
    EXPECT_TRUE(outputContains(":limituser!testuser@127.0.0.1 JOIN #test"));
    clearServerOutput();

    // Remove limit
    sendCommand(client0, "MODE #test -l");
    EXPECT_TRUE(outputContains(":basicUser0!testuser@127.0.0.1 MODE #test -l"));
    clearServerOutput();
}

// Test operator status mode
TEST_F(ModeTests, OperatorStatus)
{
    std::vector<int> clients = basicSetupMultiple(2);
    int client0 = clients[0]; // op
    int client1 = clients[1]; // regular user

    // Regular user tries to set mode (should fail)
    sendCommand(client1, "MODE #test +i");
    EXPECT_TRUE(outputContains("482 basicUser1 #test :You're not channel operator"));
    clearServerOutput();

    // Give operator status to the second user
    sendCommand(client0, "MODE #test +o basicUser1");
    EXPECT_TRUE(outputContains(":basicUser0!testuser@127.0.0.1 MODE #test +o basicUser1"));
    clearServerOutput();

    // Now second user should be able to set modes
    sendCommand(client1, "MODE #test +i");
    EXPECT_TRUE(outputContains(":basicUser1!testuser@127.0.0.1 MODE #test +i"));
    clearServerOutput();

    // Remove operator status
    sendCommand(client0, "MODE #test -o basicUser1");
    EXPECT_TRUE(outputContains(":basicUser0!testuser@127.0.0.1 MODE #test -o basicUser1"));
    clearServerOutput();

    // Should no longer be able to set modes
    sendCommand(client1, "MODE #test -i");
    EXPECT_TRUE(outputContains("482 basicUser1 #test :You're not channel operator"));
    clearServerOutput();
}

// Test multiple modes at once
TEST_F(ModeTests, MultipleModesAtOnce)
{
    std::vector<int> clients = basicSetupMultiple(2);
    int client0 = clients[0]; // op

    // Set multiple modes at once
    sendCommand(client0, "MODE #test +i+t+k testkey");
    EXPECT_TRUE(outputContains(":basicUser0!testuser@127.0.0.1 MODE #test +i"));
    EXPECT_TRUE(outputContains(":basicUser0!testuser@127.0.0.1 MODE #test +t"));
    EXPECT_TRUE(outputContains(":basicUser0!testuser@127.0.0.1 MODE #test +k testkey"));
    clearServerOutput();

    // Check that all modes are set
    sendCommand(client0, "MODE #test");
    EXPECT_TRUE(outputContains("+itk"));
    clearServerOutput();

    // Remove multiple modes at once
    sendCommand(client0, "MODE #test -i-t-k");
    EXPECT_TRUE(outputContains(":basicUser0!testuser@127.0.0.1 MODE #test -i"));
    EXPECT_TRUE(outputContains(":basicUser0!testuser@127.0.0.1 MODE #test -t"));
    EXPECT_TRUE(outputContains(":basicUser0!testuser@127.0.0.1 MODE #test -k"));
    clearServerOutput();

    // Check that modes are removed
    sendCommand(client0, "MODE #test");
    EXPECT_FALSE(outputContains("324 basicUser0 #test +i"));
    EXPECT_FALSE(outputContains("324 basicUser0 #test +t"));
    EXPECT_FALSE(outputContains("324 basicUser0 #test +k"));
    clearServerOutput();
}

// Test Nick change op permissions
TEST_F(ModeTests, NickChangeOp)
{
    std::vector<int> clients = basicSetupMultiple(2);
    int client0 = clients[0]; // op
    int client1 = clients[1]; // regular user

    // operator status stays through a nick change
    sendCommand(client0, "NICK creator");
    sendCommand(client0, "MODE #test +o basicUser1");
    // no ERR_CHANOPRIVSNEEDED message in the output
    EXPECT_FALSE(outputContains("482"));
    clearServerOutput();

    // Check that nick change didn't transfer op status by nickname
    sendCommand(client1, "NICK basicUser0");
    sendCommand(client1, "MODE #test +l 2");
    // Should still have op status because op is associated with connection, not nickname
    EXPECT_FALSE(outputContains("482"));
    clearServerOutput();
}

// Test error cases
TEST_F(ModeTests, ErrorCases)
{
    std::vector<int> clients = basicSetupMultiple(1);
    int client0 = clients[0];

    // Test invalid mode parameter
    sendCommand(client0, "MODE #test +k");
    EXPECT_TRUE(outputContains("461 basicUser0 MODE :Not enough parameters"));
    clearServerOutput();

    // Test non-existent channel
    sendCommand(client0, "MODE #nonexistent +i");
    EXPECT_TRUE(outputContains("403 basicUser0 #nonexistent :No such channel"));
    clearServerOutput();

    // Test giving op to non-existent user
    sendCommand(client0, "MODE #test +o nonexistentuser");
    EXPECT_TRUE(outputContains("401 basicUser0 nonexistentuser :No such nick"));
    clearServerOutput();

    // Test user not in channel
    int otherClient = connectClient();
    ASSERT_GT(otherClient, 0);
    registerClient(otherClient, "outsider");

    sendCommand(client0, "MODE #test +o outsider");
    EXPECT_TRUE(outputContains("441 basicUser0 outsider #test :They aren't on that channel"));
    clearServerOutput();
}
