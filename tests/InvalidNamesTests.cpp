#include "TestSetup.hpp"

class InvalidNamesTests : public TestSetup
{
protected:
    InvalidNamesTests()
        : TestSetup(true)
    {}
};

// Test invalid channel names
TEST_F(InvalidNamesTests, InvalidChannelNames)
{
    int client = connectClient();
    ASSERT_GT(client, 0);
    registerClient(client, "tester");
    clearServerOutput();

    // Channel name must start with # or &
    sendCommand(client, "JOIN nochannel");
    EXPECT_TRUE(outputContains("403") || outputContains("ERR_BADCHANMASK"));
    clearServerOutput();

    sendCommand(client, "JOIN *channel");
    EXPECT_TRUE(outputContains("403") || outputContains("ERR_BADCHANMASK"));
    clearServerOutput();

    // Channel names cannot contain spaces
    sendCommand(client, "JOIN :#invalid channel");
    EXPECT_TRUE(outputContains("403") || outputContains("ERR_BADCHANMASK"));
    clearServerOutput();

    // Channel names cannot be empty after the prefix
    sendCommand(client, "JOIN #");
    EXPECT_TRUE(outputContains("403") || outputContains("ERR_BADCHANMASK"));
    clearServerOutput();

    // Channel names cannot contain control characters
    sendCommand(client, "JOIN #test\07test"); // BEL character
    EXPECT_TRUE(outputContains("403") || outputContains("ERR_BADCHANMASK"));
    clearServerOutput();

    // Channel names cannot contain commas (used as separator)
    sendCommand(client, "JOIN #test,channel");
    // This should be treated as two different channels rather than one invalid one
    EXPECT_TRUE(outputContains("JOIN #test"));
    clearServerOutput();

    // Channel names cannot contain colons (used in messages)
    sendCommand(client, "JOIN #test:channel");
    EXPECT_TRUE(outputContains("403") || outputContains("ERR_BADCHANMASK"));
    clearServerOutput();
}

// Test channel name length limits
TEST_F(InvalidNamesTests, ChannelNameLength)
{
    int client = connectClient();
    ASSERT_GT(client, 0);
    registerClient(client, "tester");
    clearServerOutput();

    // Create a channel name that's exactly at the maximum length (CHANNELLEN from common.hpp)
    std::string maxLengthName = "#";
    maxLengthName.append(CHANNELLEN - 1, 'a'); // -1 for the # character
    sendCommand(client, "JOIN " + maxLengthName);
    EXPECT_TRUE(outputContains("JOIN " + maxLengthName));
    clearServerOutput();

    // Try to create a channel name that's too long (CHANNELLEN + 1)
    std::string tooLongName = "#";
    tooLongName.append(CHANNELLEN, 'a');
    sendCommand(client, "JOIN " + tooLongName);
    EXPECT_TRUE(outputContains("403") || outputContains("ERR_BADCHANMASK"));
    clearServerOutput();
}

// Test channel name case-insensitivity
TEST_F(InvalidNamesTests, ChannelNameCaseSensitivity)
{
    std::vector<int> clients = basicSetupMultiple(2);
    int client0 = clients[0];
    int client1 = clients[1];

    // Create a channel with uppercase letters
    sendCommand(client0, "JOIN #TestChannel");
    EXPECT_TRUE(outputContains("JOIN #TestChannel"));
    clearServerOutput();

    // Join with different case - should join the same channel
    sendCommand(client1, "JOIN #testchannel");
    // The response should show the original case from the first join
    EXPECT_TRUE(outputContains("JOIN #TestChannel"));
    clearServerOutput();

    // Send a message and verify it's received
    sendCommand(client0, "PRIVMSG #TESTCHANNEL :Hello");
    EXPECT_TRUE(outputContains("PRIVMSG #TestChannel :Hello"));
    clearServerOutput();
}

// Test valid channel names
TEST_F(InvalidNamesTests, ValidChannelNames)
{
    int client = connectClient();
    ASSERT_GT(client, 0);
    registerClient(client, "tester");
    clearServerOutput();

    // Basic valid channel names
    sendCommand(client, "JOIN #test");
    EXPECT_TRUE(outputContains("JOIN #test"));
    clearServerOutput();

    sendCommand(client, "JOIN &test");
    EXPECT_TRUE(outputContains("JOIN &test"));
    clearServerOutput();

    // Channel names can contain numbers
    sendCommand(client, "JOIN #test123");
    EXPECT_TRUE(outputContains("JOIN #test123"));
    clearServerOutput();

    // Channel names can contain special characters
    sendCommand(client, "JOIN #test-channel");
    EXPECT_TRUE(outputContains("JOIN #test-channel"));
    clearServerOutput();

    sendCommand(client, "JOIN #test_channel");
    EXPECT_TRUE(outputContains("JOIN #test_channel"));
    clearServerOutput();

    sendCommand(client, "JOIN #test.channel");
    EXPECT_TRUE(outputContains("JOIN #test.channel"));
    clearServerOutput();
}

// Test invalid nickname constraints
TEST_F(InvalidNamesTests, InvalidNicknames)
{
    int client = connectClient();
    ASSERT_GT(client, 0);
    registerClient(client, "tester");
    clearServerOutput();

    // Nickname can't start with a number
    sendCommand(client, "NICK 1user");
    EXPECT_TRUE(outputContains("432") || outputContains("ERR_ERRONEUSNICKNAME"));
    clearServerOutput();

    // Nickname can't start with special characters (except allowed ones)
    sendCommand(client, "NICK -user");
    EXPECT_TRUE(outputContains("432") || outputContains("ERR_ERRONEUSNICKNAME"));
    clearServerOutput();

    sendCommand(client, "NICK @user");
    EXPECT_TRUE(outputContains("432") || outputContains("ERR_ERRONEUSNICKNAME"));
    clearServerOutput();

    // Nickname can't contain spaces
    sendCommand(client, "NICK :test user");
    EXPECT_TRUE(outputContains("432") || outputContains("ERR_ERRONEUSNICKNAME"));
    clearServerOutput();

    // Nickname can't contain invalid special characters
    sendCommand(client, "NICK test:user");
    EXPECT_TRUE(outputContains("432") || outputContains("ERR_ERRONEUSNICKNAME"));
    clearServerOutput();

    sendCommand(client, "NICK test#user");
    EXPECT_TRUE(outputContains("432") || outputContains("ERR_ERRONEUSNICKNAME"));
    clearServerOutput();
}

// Test nickname length limits
TEST_F(InvalidNamesTests, NicknameLength)
{
    int client = connectClient();
    ASSERT_GT(client, 0);
    registerClient(client, "tester");
    clearServerOutput();

    // Create a nickname that's exactly at the maximum length (NICKLEN from common.hpp)
    std::string maxLengthNick = "a";
    maxLengthNick.append(NICKLEN - 1, 'b'); // adding NICKLEN-1 more chars
    sendCommand(client, "NICK " + maxLengthNick);
    EXPECT_TRUE(outputContains("NICK " + maxLengthNick));
    clearServerOutput();

    // Try to create a nickname that's too long (NICKLEN + 1)
    std::string tooLongNick = "a";
    tooLongNick.append(NICKLEN, 'b');
    sendCommand(client, "NICK " + tooLongNick);
    EXPECT_TRUE(outputContains("432") || outputContains("ERR_ERRONEUSNICKNAME"));
    clearServerOutput();
}

// Test valid nicknames
TEST_F(InvalidNamesTests, ValidNicknames)
{
    int client = connectClient();
    ASSERT_GT(client, 0);
    registerClient(client, "tester");
    clearServerOutput();

    // Basic valid nicknames
    sendCommand(client, "NICK validuser");
    EXPECT_TRUE(outputContains("NICK validuser"));
    clearServerOutput();

    // Nicknames can contain numbers (but not start with them)
    sendCommand(client, "NICK user123");
    EXPECT_TRUE(outputContains("NICK user123"));
    clearServerOutput();

    // Nicknames can contain certain special characters
    sendCommand(client, "NICK user_test");
    EXPECT_TRUE(outputContains("NICK user_test"));
    clearServerOutput();

    // Nicknames can start with certain special characters according to RFC
    sendCommand(client, "NICK [user]");
    EXPECT_TRUE(outputContains("NICK [user]"));
    clearServerOutput();

    sendCommand(client, "NICK \\user");
    EXPECT_TRUE(outputContains("NICK \\user"));
    clearServerOutput();

    sendCommand(client, "NICK `user");
    EXPECT_TRUE(outputContains("NICK `user"));
    clearServerOutput();

    sendCommand(client, "NICK ^user");
    EXPECT_TRUE(outputContains("NICK ^user"));
    clearServerOutput();

    sendCommand(client, "NICK {user}");
    EXPECT_TRUE(outputContains("NICK {user}"));
    clearServerOutput();

    sendCommand(client, "NICK |user|");
    EXPECT_TRUE(outputContains("NICK |user|"));
    clearServerOutput();
}

// Test nickname collision
TEST_F(InvalidNamesTests, NicknameCollision)
{
    std::vector<int> clients = basicSetupMultiple(2);
    int client0 = clients[0];
    int client1 = clients[1];

    // Try to take a nickname that's already in use
    sendCommand(client1, "NICK basicUser0");
    EXPECT_TRUE(outputContains("433") || outputContains("ERR_NICKNAMEINUSE"));
    clearServerOutput();

    // Nickname collision should be case-insensitive
    sendCommand(client1, "NICK BASICUSER0");
    EXPECT_TRUE(outputContains("433") || outputContains("ERR_NICKNAMEINUSE"));
    clearServerOutput();

    // Verify original user still has the nickname
    sendCommand(client0, "PRIVMSG basicUser1 :Hello");
    EXPECT_TRUE(outputContains(":basicUser0!testuser@127.0.0.1 PRIVMSG basicUser1 :Hello"));
    clearServerOutput();
}

// Test empty nickname
TEST_F(InvalidNamesTests, EmptyNickname)
{
    int client = connectClient();
    ASSERT_GT(client, 0);
    registerClient(client, "tester");
    clearServerOutput();

    // Try to set an empty nickname
    sendCommand(client, "NICK ");
    EXPECT_TRUE(outputContains("431") || outputContains("ERR_NONICKNAMEGIVEN"));
    clearServerOutput();
}

// Test RFC-compliant special cases that your validator might miss
TEST_F(InvalidNamesTests, SpecialCases)
{
    int client = connectClient();
    ASSERT_GT(client, 0);
    registerClient(client, "tester");
    clearServerOutput();

    // Test channel name with characters that are technically valid per RFC but might be missed
    sendCommand(client, "JOIN #test!channel");
    // If your implementation follows strict RFC, this should be accepted
    // If not, there should be an error message
    bool validResult = outputContains("JOIN #test!channel") || outputContains("403") ||
                       outputContains("ERR_BADCHANMASK");
    EXPECT_TRUE(validResult);
    clearServerOutput();

    // Test nickname with characters that are technically valid per RFC but might be missed
    sendCommand(client, "NICK }user{");
    // Again, this should be valid per RFC
    bool validNick = outputContains("NICK }user{") || outputContains("432") ||
                     outputContains("ERR_ERRONEUSNICKNAME");
    EXPECT_TRUE(validNick);
    clearServerOutput();
}
