#include "TestSetup.hpp"

class KickTests : public TestSetup
{
protected:
    KickTests()
        : TestSetup(true)
    {}
};

// Test basic kick functionality
TEST_F(KickTests, BasicKick)
{
    std::vector<int> clients = basicSetupMultiple(2);
    int client0 = clients[0]; // op
    int client1 = clients[1]; // regular user

    // Operator kicks regular user with a reason
    sendCommand(client0, "KICK #test basicUser1 :Bad behavior");
    EXPECT_TRUE(
        outputContains(":basicUser0!testuser@127.0.0.1 KICK #test basicUser1 :Bad behavior"));
    clearServerOutput();

    // Verify user is no longer in channel
    sendCommand(client1, "PRIVMSG #test :Hello");
    EXPECT_TRUE(outputContains(":basicUser1!testuser@127.0.0.1 PRIVMSG #test :Hello"));
    clearServerOutput();
}

// Test user rejoin after kick
TEST_F(KickTests, RejoinAfterKick)
{
    std::vector<int> clients = basicSetupMultiple(2);
    int client0 = clients[0]; // op
    int client1 = clients[1]; // regular user

    // Kick the user
    sendCommand(client0, "KICK #test basicUser1 :Get out");
    clearServerOutput();

    // User tries to rejoin
    sendCommand(client1, "JOIN #test");
    EXPECT_TRUE(outputContains(":basicUser1!testuser@127.0.0.1 JOIN #test"));
    clearServerOutput();
}

// Test non-operator trying to kick
TEST_F(KickTests, NonOperatorKick)
{
    std::vector<int> clients = basicSetupMultiple(3);
    int client0 = clients[0]; // op
    int client1 = clients[1]; // regular user trying to kick
    int client2 = clients[2]; // user to be kicked

    // Mark client2 as used to avoid unused variable warning
    (void)client2;

    // Non-operator tries to kick someone
    sendCommand(client1, "KICK #test basicUser2 :Not allowed");
    EXPECT_TRUE(outputContains("482 basicUser1 #test :You're not channel operator"));
    clearServerOutput();

    // Make client1 an operator
    sendCommand(client0, "MODE #test +o basicUser1");
    clearServerOutput();

    // Now client1 should be able to kick
    sendCommand(client1, "KICK #test basicUser2 :Now I can kick");
    EXPECT_TRUE(
        outputContains(":basicUser1!testuser@127.0.0.1 KICK #test basicUser2 :Now I can kick"));
    clearServerOutput();
}

// Test kicking multiple users at once
TEST_F(KickTests, MultipleKicks)
{
    std::vector<int> clients = basicSetupMultiple(3);
    int client0 = clients[0]; // op

    // Kick multiple users at once
    sendCommand(client0, "KICK #test basicUser1,basicUser2 :Goodbye everyone");
    EXPECT_TRUE(
        outputContains(":basicUser0!testuser@127.0.0.1 KICK #test basicUser1 :Goodbye everyone"));
    EXPECT_TRUE(
        outputContains(":basicUser0!testuser@127.0.0.1 KICK #test basicUser2 :Goodbye everyone"));
    clearServerOutput();
}

// Test kicking from a channel the kicker is not on
TEST_F(KickTests, KickerNotInChannel)
{
    std::vector<int> clients = basicSetupMultiple(2);
    int client0 = clients[0]; // op in #test
    int client1 = clients[1]; // regular user

    // Create a second channel
    sendCommand(client1, "JOIN #secondchan");
    clearServerOutput();

    // Client0 tries to kick from a channel they're not on
    sendCommand(client0, "KICK #secondchan basicUser1 :Revenge");
    EXPECT_TRUE(outputContains("442 basicUser0 #secondchan :You're not on that channel"));
    clearServerOutput();
}

// Test kicking a user who is not on the channel
TEST_F(KickTests, KickeeNotInChannel)
{
    std::vector<int> clients = basicSetupMultiple(2);
    int client0 = clients[0]; // op
    int client1 = clients[1]; // regular user

    // Mark client1 as used to avoid unused variable warning
    (void)client1;

    // Create a second channel that client1 doesn't join
    sendCommand(client0, "JOIN #secondchan");
    clearServerOutput();

    // Try to kick a user who isn't on the channel
    sendCommand(client0, "KICK #secondchan basicUser1 :Not here");
    EXPECT_TRUE(
        outputContains("441 basicUser0 basicUser1 #secondchan :They aren't on that channel"));
    clearServerOutput();
}

// Test kicking a non-existent user
TEST_F(KickTests, NonExistentUser)
{
    std::vector<int> clients = basicSetupMultiple(1);
    int client0 = clients[0]; // op

    // Try to kick a nonexistent user
    sendCommand(client0, "KICK #test nonexistentuser :Who are you?");
    EXPECT_TRUE(outputContains("401 basicUser0 nonexistentuser :No such nick"));
    clearServerOutput();
}

// Test kicking from a non-existent channel
TEST_F(KickTests, NonExistentChannel)
{
    std::vector<int> clients = basicSetupMultiple(2);
    int client0 = clients[0]; // op

    // Try to kick from a non-existent channel
    sendCommand(client0, "KICK #nonexistent basicUser1 :No channel");
    EXPECT_TRUE(outputContains("403 basicUser0 #nonexistent :No such channel"));
    clearServerOutput();
}

// Test kick without enough parameters
TEST_F(KickTests, NotEnoughParameters)
{
    std::vector<int> clients = basicSetupMultiple(1);
    int client0 = clients[0]; // op

    // Try to kick without specifying who
    sendCommand(client0, "KICK #test");
    EXPECT_TRUE(outputContains("461 basicUser0 KICK :Not enough parameters"));
    clearServerOutput();

    // Try to kick without specifying a channel
    sendCommand(client0, "KICK");
    EXPECT_TRUE(outputContains("461 basicUser0 KICK :Not enough parameters"));
    clearServerOutput();
}
