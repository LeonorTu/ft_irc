#include <gtest/gtest.h>
#include <Server.hpp>
#include <thread>
#include <chrono>
#include <mutex>
#include <atomic>
#include <vector>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <cstring>
#include <common.hpp>
#include <sstream>
#include <fcntl.h>
#include <ChannelManager.hpp>
#include "TestSetup.hpp"

// Test successful nickname change
TEST_F(TestSetup, NickCommandSuccess)
{
    // Register first client
    int client = connectClient();
    registerClient(client, "user1");

    // Change nickname
    sendCommand(client, "NICK newname");

    // Wait for response
    std::this_thread::sleep_for(std::chrono::milliseconds(10));

    std::cerr << getServerOutput() << std::endl;
    // Check if nickname was changed successfully
    EXPECT_TRUE(outputContains(":user1!testuser@127.0.0.1 NICK newname"));
}

// Test nickname collision
TEST_F(TestSetup, NickCommandCollision)
{
    // Register both clients with different nicknames
    int client1 = connectClient();
    int client2 = connectClient();
    clearServerOutput();

    registerClient(client1, "user1");
    clearServerOutput();
    std::this_thread::sleep_for(std::chrono::milliseconds(40));
    registerClient(client2, "user2");
    clearServerOutput();

    // Try to change second client to first client's nickname
    sendCommand(client2, "NICK user1");

    // Wait for response
    std::this_thread::sleep_for(std::chrono::milliseconds(10));

    // Check if error message was sent
    EXPECT_TRUE(outputContains("433"));
}

// Test invalid nickname
TEST_F(TestSetup, NickCommandInvalidName)
{
    // Register client
    int client = connectClient();
    registerClient(client, "user1");

    // Try to change to invalid nickname (starts with number)
    sendCommand(client, "NICK 1invalid");

    // Wait for response
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    // Check if error message was sent
    EXPECT_TRUE(outputContains("432 user1 1invalid"));
}

// Test nickname too long
TEST_F(TestSetup, NickCommandTooLong)
{
    // Register client
    int client = connectClient();
    registerClient(client, "user1");

    // Generate a nickname that exceeds NICKLEN
    std::string longNick(NICKLEN + 5, 'a');

    // Try to change to long nickname
    sendCommand(client, "NICK " + longNick);

    // Wait for response
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    // Either it gets truncated or rejected - verify current nickname isn't the full long one
    EXPECT_FALSE(outputContains(":" + longNick));

    // Check that either an error was sent or a truncated version was accepted
    bool success = outputContains("NICK " + longNick.substr(0, NICKLEN)) ||
                   outputContains("432 user1 " + longNick);
    EXPECT_TRUE(success);
}

TEST_F(TestSetup, JOINCommand)
{
    // Register both clients with different nicknames
    int client1 = connectClient();
    int client2 = connectClient();

    // Make sure clients are connected successfully
    ASSERT_GT(client1, 0);
    ASSERT_GT(client2, 0);

    registerClient(client1, "user1");
    registerClient(client2, "user2");

    sendCommand(client1, "JOIN #test");
    EXPECT_TRUE(outputContains(":user1!testuser@127.0.0.1 JOIN #test"));
    clearServerOutput();

    sendCommand(client2, "JOIN #test");
    EXPECT_TRUE(outputContains(":user2!testuser@127.0.0.1 JOIN #test"));
    clearServerOutput();
}

TEST_F(TestSetup, TestTopic)
{
    // Register both clients with different nicknames
    int client1 = connectClient();
    int client2 = connectClient();

    // Make sure clients are connected successfully
    ASSERT_GT(client1, 0);
    ASSERT_GT(client2, 0);
    registerClient(client1, "user1");
    registerClient(client2, "user2");
    sendCommand(client1, "JOIN #test");
    sendCommand(client2, "JOIN #test");
    clearServerOutput();
    sendCommand(client1, "TOPIC #test :mew topic");

    EXPECT_TRUE(outputContains("user1!testuser@127.0.0.1 TOPIC #test :mew topic"));
    clearServerOutput();
}

TEST_F(TestSetup, InviteCommand)
{
    // Register both clients with different nicknames
    int client1 = connectClient();
    int client2 = connectClient();

    // Make sure clients are connected successfully
    ASSERT_GT(client1, 0);
    ASSERT_GT(client2, 0);

    // Register clients
    registerClient(client1, "user1");
    registerClient(client2, "user2");

    // Client1 creates and joins a channel
    sendCommand(client1, "JOIN #invitetest");
    EXPECT_TRUE(outputContains(":user1!testuser@127.0.0.1 JOIN #invitetest"));
    clearServerOutput();

    // Client1 invites client2 to the channel
    sendCommand(client1, "INVITE user2 #invitetest");

    // Check for proper invite notification
    EXPECT_TRUE(outputContains("341 user1 user2 #invitetest")); // RPL_INVITING to user1
    EXPECT_TRUE(
        outputContains(":user1!testuser@127.0.0.1 INVITE user2 #invitetest")); // INVITE to user2
    clearServerOutput();

    // Client2 joins the channel using the invitation
    sendCommand(client2, "JOIN #invitetest");
    EXPECT_TRUE(outputContains(":user2!testuser@127.0.0.1 JOIN #invitetest"));
    clearServerOutput();

    // Test error case: Try to invite client2 again (already in channel)
    sendCommand(client1, "INVITE user2 #invitetest");
    EXPECT_TRUE(outputContains("443 user1 user2 #invitetest :is already on channel"));
    clearServerOutput();

    // Test error case: Client2 tries to invite non-existent user
    sendCommand(client2, "INVITE nonexistentuser #invitetest");
    EXPECT_TRUE(outputContains("401 user2 nonexistentuser :No such nick"));
    clearServerOutput();

    // Test invite-only channel
    // First, set the channel to invite-only
    sendCommand(client1, "MODE #invitetest +i");
    clearServerOutput();

    // Have client2 part the channel, then try to rejoin (should fail without invite)
    sendCommand(client2, "PART #invitetest");
    clearServerOutput();

    sendCommand(client2, "JOIN #invitetest");
    EXPECT_TRUE(outputContains("473 user2 #invitetest :Cannot join channel (+i)"));
    clearServerOutput();

    // Have client1 (who's still in the channel) invite client2
    sendCommand(client1, "INVITE user2 #invitetest");
    clearServerOutput();

    // Now client2 should be able to join
    sendCommand(client2, "JOIN #invitetest");
    EXPECT_TRUE(outputContains(":user2!testuser@127.0.0.1 JOIN #invitetest"));
    clearServerOutput();
}

TEST_F(TestSetup, KickCommand)
{
    // Register three clients with different nicknames
    int client1 = connectClient();
    int client2 = connectClient();
    int client3 = connectClient();

    // Make sure clients are connected successfully
    ASSERT_GT(client1, 0);
    ASSERT_GT(client2, 0);
    ASSERT_GT(client3, 0);

    // Register clients
    registerClient(client1, "user1");
    registerClient(client2, "user2");
    registerClient(client3, "user3");

    // Client1 creates and joins a channel (becomes operator)
    sendCommand(client1, "JOIN #kicktest");
    EXPECT_TRUE(outputContains(":user1!testuser@127.0.0.1 JOIN #kicktest"));
    clearServerOutput();

    // Client2 and Client3 join the channel
    sendCommand(client2, "JOIN #kicktest");
    EXPECT_TRUE(outputContains(":user2!testuser@127.0.0.1 JOIN #kicktest"));
    clearServerOutput();

    sendCommand(client3, "JOIN #kicktest");
    EXPECT_TRUE(outputContains(":user3!testuser@127.0.0.1 JOIN #kicktest"));
    clearServerOutput();

    // Client1 kicks Client2 from the channel with a reason
    sendCommand(client1, "KICK #kicktest user2 :Bad behavior");
    EXPECT_TRUE(outputContains(":user1!testuser@127.0.0.1 KICK #kicktest user2 :Bad behavior"));
    clearServerOutput();

    // Client2 tries to rejoin
    sendCommand(client2, "JOIN #kicktest");
    EXPECT_TRUE(outputContains(":user2!testuser@127.0.0.1 JOIN #kicktest"));
    clearServerOutput();

    // Client3 tries to kick Client2 (should fail as non-operator)
    sendCommand(client3, "KICK #kicktest user2 :Not allowed");
    EXPECT_TRUE(outputContains("482 user3 #kicktest :You're not channel operator"));
    clearServerOutput();

    // Client1 kicks multiple users at once
    sendCommand(client1, "KICK #kicktest user2,user3 :Goodbye everyone");
    EXPECT_TRUE(outputContains(":user1!testuser@127.0.0.1 KICK #kicktest user2 :Goodbye everyone"));
    EXPECT_TRUE(outputContains(":user1!testuser@127.0.0.1 KICK #kicktest user3 :Goodbye everyone"));
    clearServerOutput();

    // Create a second channel for additional tests
    sendCommand(client1, "JOIN #secondchan");
    clearServerOutput();

    // Client2 tries to kick from a channel they're not on
    sendCommand(client2, "KICK #secondchan user1 :Revenge");
    EXPECT_TRUE(outputContains("442 user2 #secondchan :You're not on that channel"));
    clearServerOutput();

    // Client1 tries to kick a user not on the channel
    sendCommand(client1, "KICK #secondchan user2 :Not here");
    EXPECT_TRUE(outputContains("441 user1 user2 #secondchan :They aren't on that channel"));
    clearServerOutput();

    // Client1 tries to kick a nonexistent user
    sendCommand(client1, "KICK #kicktest nonexistentuser :Who are you?");
    EXPECT_TRUE(outputContains("401 user1 nonexistentuser :No such nick"));
    clearServerOutput();
}

TEST_F(TestSetup, ModeCommand)
{
    // Register three clients with different nicknames
    int client1 = connectClient();
    int client2 = connectClient();
    int client3 = connectClient();

    // Make sure clients are connected successfully
    ASSERT_GT(client1, 0);
    ASSERT_GT(client2, 0);
    ASSERT_GT(client3, 0);

    // Register clients
    registerClient(client1, "user1");
    registerClient(client2, "user2");
    registerClient(client3, "user3");

    // Client1 creates and joins a channel (becomes operator)
    sendCommand(client1, "JOIN #modetest");
    EXPECT_TRUE(outputContains(":user1!testuser@127.0.0.1 JOIN #modetest"));
    clearServerOutput();

    // Client2 joins the channel
    sendCommand(client2, "JOIN #modetest");
    EXPECT_TRUE(outputContains(":user2!testuser@127.0.0.1 JOIN #modetest"));
    clearServerOutput();

    // Test viewing channel modes (should show default modes)
    sendCommand(client1, "MODE #modetest");
    EXPECT_TRUE(outputContains("324 user1 #modetest"));
    clearServerOutput();

    // Test setting invite-only mode
    sendCommand(client1, "MODE #modetest +i");
    EXPECT_TRUE(outputContains(":user1!testuser@127.0.0.1 MODE #modetest +i"));
    clearServerOutput();

    // Test client3 trying to join (should fail due to +i)
    sendCommand(client3, "JOIN #modetest");
    EXPECT_TRUE(outputContains("473 user3 #modetest :Cannot join channel (+i)"));
    clearServerOutput();

    // Test setting channel key
    sendCommand(client1, "MODE #modetest +k testkey");
    EXPECT_TRUE(outputContains(":user1!testuser@127.0.0.1 MODE #modetest +k testkey"));
    clearServerOutput();

    // Test client3 trying to join with wrong key
    sendCommand(client1, "INVITE user3 #modetest");
    sendCommand(client3, "JOIN #modetest wrongkey");
    EXPECT_TRUE(outputContains("475 user3 #modetest :Cannot join channel (+k)"));
    clearServerOutput();

    // Test client3 joining with correct key
    sendCommand(client3, "JOIN #modetest testkey");
    EXPECT_TRUE(outputContains(":user3!testuser@127.0.0.1 JOIN #modetest"));
    clearServerOutput();

    // Test protected topic mode
    sendCommand(client1, "MODE #modetest +t");
    EXPECT_TRUE(outputContains(":user1!testuser@127.0.0.1 MODE #modetest +t"));
    clearServerOutput();

    // Test non-operator trying to change topic (should fail due to +t)
    sendCommand(client2, "TOPIC #modetest :This is a new topic");
    EXPECT_TRUE(outputContains("482 user2 #modetest :You're not channel operator"));
    clearServerOutput();

    // Test setting user limit
    sendCommand(client1, "MODE #modetest +l 3");
    EXPECT_TRUE(outputContains(":user1!testuser@127.0.0.1 MODE #modetest +l 3"));
    clearServerOutput();

    // Test client4 trying to join (should fail due to +l)
    int client4 = connectClient();
    ASSERT_GT(client4, 0);
    registerClient(client4, "user4");
    sendCommand(client1, "INVITE user4 #modetest");
    sendCommand(client4, "JOIN #modetest testkey");
    EXPECT_TRUE(outputContains("471 user4 #modetest :Cannot join channel (+l)"));
    clearServerOutput();

    // Test giving operator status
    sendCommand(client1, "MODE #modetest +o user2");
    EXPECT_TRUE(outputContains(":user1!testuser@127.0.0.1 MODE #modetest +o user2"));
    clearServerOutput();

    // Test new operator changing topic (should now succeed)
    sendCommand(client2, "TOPIC #modetest :This is a new topic");
    EXPECT_TRUE(outputContains(":user2!testuser@127.0.0.1 TOPIC #modetest :This is a new topic"));
    clearServerOutput();

    // Test removing operator status
    sendCommand(client1, "MODE #modetest -o user2");
    EXPECT_TRUE(outputContains(":user1!testuser@127.0.0.1 MODE #modetest -o user2"));
    clearServerOutput();

    // Test removing modes
    sendCommand(client1, "MODE #modetest -i-t-k-l");
    EXPECT_TRUE(outputContains(":user1!testuser@127.0.0.1 MODE #modetest -i"));
    EXPECT_TRUE(outputContains(":user1!testuser@127.0.0.1 MODE #modetest -t"));
    EXPECT_TRUE(outputContains(":user1!testuser@127.0.0.1 MODE #modetest -k"));
    EXPECT_TRUE(outputContains(":user1!testuser@127.0.0.1 MODE #modetest -l"));
    clearServerOutput();

    // Test viewing channel modes again (should show empty or default modes)
    sendCommand(client1, "MODE #modetest");
    EXPECT_TRUE(outputContains("324 user1 #modetest"));
    EXPECT_FALSE(outputContains("324 user1 #modetest +i"));
    EXPECT_FALSE(outputContains("324 user1 #modetest +t"));
    clearServerOutput();

    // Test non-operator trying to set modes
    sendCommand(client2, "MODE #modetest +i");
    EXPECT_TRUE(outputContains("482 user2 #modetest :You're not channel operator"));
    clearServerOutput();

    // Test invalid mode parameter
    sendCommand(client1, "MODE #modetest +k");
    EXPECT_TRUE(outputContains("461 user1 MODE :Not enough parameters"));
    clearServerOutput();
}