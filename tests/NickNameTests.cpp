#include "TestSetup.hpp"

class NickNameTests : public TestSetup
{
protected:
    // Constructor with verbose flag
    NickNameTests()
        : TestSetup(true)
    {}
};

// Test successful nickname change
TEST_F(NickNameTests, Success)
{
    // Use basicSetupMultiple for standard setup
    std::vector<int> clients = basicSetupMultiple(1);
    int client0 = clients[0];

    // Change nickname
    sendCommand(client0, "NICK newname");

    // Check if nickname was changed successfully
    EXPECT_TRUE(outputContains(":basicUser0!testuser@127.0.0.1 NICK newname"));
}

// Test nickname collision
TEST_F(NickNameTests, Collision)
{
    // Use basicSetupMultiple for standard setup
    std::vector<int> clients = basicSetupMultiple(2);
    int client0 = clients[0];
    int client1 = clients[1];

    // Mark client0 as used to avoid unused variable warning
    (void)client0;

    clearServerOutput();

    // Try to change second client to first client's nickname
    sendCommand(client1, "NICK basicUser0");

    // Check if error message was sent
    EXPECT_TRUE(outputContains("433"));
}

// Test invalid nickname
TEST_F(NickNameTests, InvalidName)
{
    // Use basicSetupMultiple for standard setup
    std::vector<int> clients = basicSetupMultiple(1);
    int client0 = clients[0];

    // Try to change to invalid nickname (starts with number)
    sendCommand(client0, "NICK 1invalid");

    // Check if error message was sent
    EXPECT_TRUE(outputContains("432 basicUser0 1invalid"));
}

// Test nickname too long
TEST_F(NickNameTests, TooLong)
{
    // Use basicSetupMultiple for standard setup
    std::vector<int> clients = basicSetupMultiple(1);
    int client0 = clients[0];

    // Generate a nickname that exceeds NICKLEN
    std::string longNick(NICKLEN + 5, 'a');

    // Try to change to long nickname
    sendCommand(client0, "NICK " + longNick);

    // Either it gets truncated or rejected - verify current nickname isn't the full long one
    EXPECT_FALSE(outputContains(":" + longNick));

    // Check that either an error was sent or a truncated version was accepted
    bool success = outputContains("NICK " + longNick.substr(0, NICKLEN)) ||
                   outputContains("432 basicUser0 " + longNick);
    EXPECT_TRUE(success);
}
