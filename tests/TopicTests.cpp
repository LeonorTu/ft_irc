#include "TestSetup.hpp"

class TopicTests : public TestSetup
{
protected:
    TopicTests()
        : TestSetup(true)
    {}
};

// Test basic topic setting and viewing
TEST_F(TopicTests, BasicTopic)
{
    std::vector<int> clients = basicSetupMultiple(2);
    int client1 = clients[0];
    int client2 = clients[1];

    // Check that no topic is set initially
    sendCommand(client1, "TOPIC #test");
    EXPECT_TRUE(outputContains("331 basicUser0 #test :No topic is set"));
    clearServerOutput();

    // Set a topic
    sendCommand(client1, "TOPIC #test :This is a test topic");
    EXPECT_TRUE(outputContains(":basicUser0!testuser@127.0.0.1 TOPIC #test :This is a test topic"));
    clearServerOutput();

    // View the topic
    sendCommand(client2, "TOPIC #test");
    EXPECT_TRUE(outputContains("332 basicUser1 #test :This is a test topic"));
    EXPECT_TRUE(outputContains("333 basicUser1 #test basicUser0")); // Topic setter info
    clearServerOutput();
}

// Test topic with protected mode (+t)
TEST_F(TopicTests, ProtectedTopic)
{
    std::vector<int> clients = basicSetupMultiple(2);
    int client1 = clients[0]; // op
    int client2 = clients[1]; // regular user

    // Set protected topic mode
    sendCommand(client1, "MODE #test +t");
    EXPECT_TRUE(outputContains(":basicUser0!testuser@127.0.0.1 MODE #test +t"));
    clearServerOutput();

    // Non-op tries to change topic
    sendCommand(client2, "TOPIC #test :This should fail");
    EXPECT_TRUE(outputContains("482 basicUser1 #test :You're not channel operator"));
    clearServerOutput();

    // Op can change topic
    sendCommand(client1, "TOPIC #test :Only ops can change this");
    EXPECT_TRUE(
        outputContains(":basicUser0!testuser@127.0.0.1 TOPIC #test :Only ops can change this"));
    clearServerOutput();

    // Make client2 an op
    sendCommand(client1, "MODE #test +o basicUser1");
    clearServerOutput();

    // Now client2 should be able to change topic
    sendCommand(client2, "TOPIC #test :I am an op now");
    EXPECT_TRUE(outputContains(":basicUser1!testuser@127.0.0.1 TOPIC #test :I am an op now"));
    clearServerOutput();
}

// Test empty topic (clearing the topic)
TEST_F(TopicTests, EmptyTopic)
{
    std::vector<int> clients = basicSetupMultiple(1);
    int client = clients[0];

    // Set a topic first
    sendCommand(client, "TOPIC #test :Initial topic");
    clearServerOutput();

    // Clear the topic
    sendCommand(client, "TOPIC #test :");
    EXPECT_TRUE(outputContains(":basicUser0!testuser@127.0.0.1 TOPIC #test :"));
    clearServerOutput();

    // Verify it's cleared
    sendCommand(client, "TOPIC #test");
    EXPECT_TRUE(outputContains("331 basicUser0 #test :No topic is set"));
    clearServerOutput();
}

// Test non-existent channel and user not in channel
TEST_F(TopicTests, ErrorCases)
{
    std::vector<int> clients = basicSetupMultiple(1);
    int client1 = clients[0];

    // Create a new client that's not in any channel
    int client2 = connectClient();
    ASSERT_GT(client2, 0);
    registerClient(client2, "outsider");
    clearServerOutput();

    // Non-existent channel
    sendCommand(client1, "TOPIC #nonexistent");
    EXPECT_TRUE(outputContains("403 basicUser0 #nonexistent :No such channel"));
    clearServerOutput();

    // User not in channel tries to view topic
    sendCommand(client2, "TOPIC #test");
    EXPECT_TRUE(outputContains("442 outsider #test :You're not on that channel"));
    clearServerOutput();

    // User not in channel tries to set topic
    sendCommand(client2, "TOPIC #test :Outsider topic");
    EXPECT_TRUE(outputContains("442 outsider #test :You're not on that channel"));
    clearServerOutput();
}
