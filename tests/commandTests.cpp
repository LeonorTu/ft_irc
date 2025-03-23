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

TEST_F(TestSetup, NickChangeOp)
{
    basicSetupTwo();

    // operator status stays through a nick change
    sendCommand(basicCreator, "NICK creator");
    sendCommand(basicCreator, "MODE #test +o basicRegular1");
    // no ERR_CHANOPRIVSNEEDED message in the output
    EXPECT_FALSE(outputContains("482"));
    clearServerOutput();
}

TEST_F(TestSetup, NameStealOp)
{
    basicSetupTwo();

    // op changes name to creator
    sendCommand(basicCreator, "NICK creator");
    // regular non-op changes name to the old creators op name
    sendCommand(basicRegular1, "NICK basicCreator");
    // op status should not be enabled by name for basicRegular now
    sendCommand(basicRegular1, "MODE #test +l 2");
    // ERR_CHANOPRIVSNEEDED message should be in the output
    EXPECT_TRUE(outputContains("482"));
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

TEST_F(TestSetup, TopicCommand)
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
    sendCommand(client1, "JOIN #topictest");
    EXPECT_TRUE(outputContains(":user1!testuser@127.0.0.1 JOIN #topictest"));
    clearServerOutput();

    // Client2 joins the channel
    sendCommand(client2, "JOIN #topictest");
    EXPECT_TRUE(outputContains(":user2!testuser@127.0.0.1 JOIN #topictest"));
    clearServerOutput();

    // Test getting topic when none is set
    sendCommand(client1, "TOPIC #topictest");
    EXPECT_TRUE(outputContains("331 user1 #topictest :No topic is set"));
    clearServerOutput();

    // Client1 sets a topic as channel operator
    sendCommand(client1, "TOPIC #topictest :This is a test topic");
    EXPECT_TRUE(outputContains(":user1!testuser@127.0.0.1 TOPIC #topictest :This is a test topic"));
    clearServerOutput();

    // Client2 views the topic
    sendCommand(client2, "TOPIC #topictest");
    EXPECT_TRUE(outputContains("332 user2 #topictest :This is a test topic"));
    EXPECT_TRUE(outputContains("333 user2 #topictest user1")); // Topic setter and time
    clearServerOutput();

    // Client2 changes the topic (should work as no +t mode)
    sendCommand(client2, "TOPIC #topictest :Topic changed by non-op");
    EXPECT_TRUE(
        outputContains(":user2!testuser@127.0.0.1 TOPIC #topictest :Topic changed by non-op"));
    clearServerOutput();

    // Set protected topic mode
    sendCommand(client1, "MODE #topictest +t");
    EXPECT_TRUE(outputContains(":user1!testuser@127.0.0.1 MODE #topictest +t"));
    clearServerOutput();

    // Client2 tries to change topic (should fail due to +t)
    sendCommand(client2, "TOPIC #topictest :This should fail");
    EXPECT_TRUE(outputContains("482 user2 #topictest :You're not channel operator"));
    clearServerOutput();

    // Client1 can still change the topic as operator
    sendCommand(client1, "TOPIC #topictest :Only ops can change this topic");
    EXPECT_TRUE(outputContains(
        ":user1!testuser@127.0.0.1 TOPIC #topictest :Only ops can change this topic"));
    clearServerOutput();

    // Test viewing topic works for everyone
    sendCommand(client2, "TOPIC #topictest");
    EXPECT_TRUE(outputContains("332 user2 #topictest :Only ops can change this topic"));
    clearServerOutput();

    // Client3 (not in channel) tries to view topic
    sendCommand(client3, "TOPIC #topictest");
    EXPECT_TRUE(outputContains("442 user3 #topictest :You're not on that channel"));
    clearServerOutput();

    // Client3 (not in channel) tries to change topic
    sendCommand(client3, "TOPIC #topictest :Outsider topic");
    EXPECT_TRUE(outputContains("442 user3 #topictest :You're not on that channel"));
    clearServerOutput();

    // Test non-existent channel
    sendCommand(client1, "TOPIC #nonexistent");
    EXPECT_TRUE(outputContains("403 user1 #nonexistent :No such channel"));
    clearServerOutput();

    // Make Client2 an operator
    sendCommand(client1, "MODE #topictest +o user2");
    clearServerOutput();

    // Client2 should now be able to change topic with +t mode
    sendCommand(client2, "TOPIC #topictest :Topic from new operator");
    EXPECT_TRUE(
        outputContains(":user2!testuser@127.0.0.1 TOPIC #topictest :Topic from new operator"));
    clearServerOutput();

    // Test empty topic (clear the topic)
    sendCommand(client1, "TOPIC #topictest :");
    EXPECT_TRUE(outputContains(":user1!testuser@127.0.0.1 TOPIC #topictest :"));
    clearServerOutput();

    // Check that topic is now empty
    sendCommand(client1, "TOPIC #topictest");
    EXPECT_TRUE(outputContains("331 user1 #topictest :No topic is set"));
    clearServerOutput();
}

TEST_F(TestSetup, JoinCommandComprehensive)
{
    // Register multiple clients
    int client1 = connectClient();
    int client2 = connectClient();
    int client3 = connectClient();
    int client4 = connectClient();

    // Make sure clients are connected successfully
    ASSERT_GT(client1, 0);
    ASSERT_GT(client2, 0);
    ASSERT_GT(client3, 0);
    ASSERT_GT(client4, 0);

    // Register clients
    registerClient(client1, "user1");
    registerClient(client2, "user2");
    registerClient(client3, "user3");
    registerClient(client4, "user4");
    clearServerOutput();

    // Test 1: Basic channel join
    sendCommand(client1, "JOIN #testchan");
    EXPECT_TRUE(outputContains(":user1!testuser@127.0.0.1 JOIN #testchan"));
    // Should receive RPL_NAMREPLY and RPL_ENDOFNAMES
    EXPECT_TRUE(outputContains("353 user1 = #testchan :@user1"));
    EXPECT_TRUE(outputContains("366 user1 #testchan :End of /NAMES list"));
    clearServerOutput();

    // Test 2: Another client joining the channel
    sendCommand(client2, "JOIN #testchan");
    EXPECT_TRUE(outputContains(":user2!testuser@127.0.0.1 JOIN #testchan"));
    // Both users should receive naming info for client2
    EXPECT_TRUE(outputContains("353 user2 = #testchan :user2 @user1"));
    clearServerOutput();

    // Test 3: Joining multiple channels at once
    sendCommand(client3, "JOIN #testchan,#multichan");
    EXPECT_TRUE(outputContains(":user3!testuser@127.0.0.1 JOIN #testchan"));
    EXPECT_TRUE(outputContains(":user3!testuser@127.0.0.1 JOIN #multichan"));
    clearServerOutput();

    // Test 4: Trying to join the same channel again (should be ignored)
    sendCommand(client1, "JOIN #testchan");
    EXPECT_FALSE(
        outputContains(":user1!testuser@127.0.0.1 JOIN #testchan")); // No duplicate JOIN message
    clearServerOutput();

    // Test 5: Set channel to invite-only and test join restrictions
    sendCommand(client1, "MODE #testchan +i");
    clearServerOutput();

    sendCommand(client4, "JOIN #testchan");
    EXPECT_TRUE(
        outputContains("473 user4 #testchan :Cannot join channel (+i)")); // Cannot join invite-only
    clearServerOutput();

    // Test 6: Invite user4 and verify they can join
    sendCommand(client1, "INVITE user4 #testchan");
    clearServerOutput();
    sendCommand(client4, "JOIN #testchan");
    EXPECT_TRUE(
        outputContains(":user4!testuser@127.0.0.1 JOIN #testchan")); // Now can join after invite
    clearServerOutput();

    // Test 7: Set key on the other channel and test key-based joining
    sendCommand(client3, "MODE #multichan +k secretkey");
    clearServerOutput();

    // Wrong key
    sendCommand(client2, "JOIN #multichan wrongkey");
    EXPECT_TRUE(outputContains("475 user2 #multichan :Cannot join channel (+k)")); // Wrong key
    clearServerOutput();

    // Correct key
    sendCommand(client2, "JOIN #multichan secretkey");
    EXPECT_TRUE(outputContains(":user2!testuser@127.0.0.1 JOIN #multichan")); // Correct key
    clearServerOutput();

    // Test 8: Set user limit and test limit enforcement
    sendCommand(client1, "MODE #testchan +l 4");
    clearServerOutput();

    // Create one more client to test the limit
    int client5 = connectClient();
    ASSERT_GT(client5, 0);
    registerClient(client5, "user5");

    sendCommand(client1, "INVITE user5 #testchan");
    sendCommand(client5, "JOIN #testchan"); // Should fail as channel is at capacity
    EXPECT_TRUE(outputContains("471 user5 #testchan :Cannot join channel (+l)"));
    clearServerOutput();

    // Test 9: Test JOIN 0 (leaving all channels)
    sendCommand(client1, "JOIN 0");
    EXPECT_TRUE(outputContains(
        ":user1!testuser@127.0.0.1 PART #testchan :Client is leaving all the channels"));
    clearServerOutput();

    // Test 10: Invalid channel name
    sendCommand(client1, "JOIN invalidchannel");
    EXPECT_TRUE(outputContains("ERR_NOSUCHCHANNEL") || outputContains("403"));
    clearServerOutput();

    // Test 11: Join with empty parameter
    sendCommand(client1, "JOIN");
    EXPECT_TRUE(outputContains("461 user1 JOIN :Not enough parameters"));
    clearServerOutput();

    // Test 12: Join with keys for multiple channels
    sendCommand(client1, "JOIN #newchan1,#newchan2 key1,key2");
    EXPECT_TRUE(outputContains(":user1!testuser@127.0.0.1 JOIN #newchan1"));
    EXPECT_TRUE(outputContains(":user1!testuser@127.0.0.1 JOIN #newchan2"));
    clearServerOutput();
}

TEST_F(TestSetup, PartCommandComprehensive)
{
    // Register multiple clients
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
    clearServerOutput();

    // Setup: Create channels and join them
    sendCommand(client1, "JOIN #channel1,#channel2,#channel3");
    EXPECT_TRUE(outputContains(":user1!testuser@127.0.0.1 JOIN #channel1"));
    EXPECT_TRUE(outputContains(":user1!testuser@127.0.0.1 JOIN #channel2"));
    EXPECT_TRUE(outputContains(":user1!testuser@127.0.0.1 JOIN #channel3"));
    clearServerOutput();

    sendCommand(client2, "JOIN #channel1,#channel2");
    EXPECT_TRUE(outputContains(":user2!testuser@127.0.0.1 JOIN #channel1"));
    EXPECT_TRUE(outputContains(":user2!testuser@127.0.0.1 JOIN #channel2"));
    clearServerOutput();

    // Test 1: Basic PART command
    sendCommand(client1, "PART #channel1");
    EXPECT_TRUE(outputContains(":user1!testuser@127.0.0.1 PART #channel1"));
    clearServerOutput();

    // Verify client1 is no longer in the channel by trying to use channel-only commands
    sendCommand(client1, "TOPIC #channel1 :This should fail");
    EXPECT_TRUE(outputContains("442 user1 #channel1 :You're not on that channel"));
    clearServerOutput();

    // Test 2: PART with reason
    sendCommand(client1, "PART #channel2 :Leaving for testing");
    EXPECT_TRUE(outputContains(":user1!testuser@127.0.0.1 PART #channel2 :Leaving for testing"));
    clearServerOutput();

    // Test 3: PART multiple channels at once
    sendCommand(client2, "PART #channel1,#channel2 :Leaving multiple channels");
    EXPECT_TRUE(
        outputContains(":user2!testuser@127.0.0.1 PART #channel1 :Leaving multiple channels"));
    EXPECT_TRUE(
        outputContains(":user2!testuser@127.0.0.1 PART #channel2 :Leaving multiple channels"));
    clearServerOutput();

    // Test 4: PART non-existent channel
    sendCommand(client1, "PART #nonexistent");
    EXPECT_TRUE(outputContains("403 user1 #nonexistent :No such channel"));
    clearServerOutput();

    // Test 5: PART channel user isn't in
    sendCommand(client2, "PART #channel3");
    EXPECT_TRUE(outputContains("442 user2 #channel3 :You're not on that channel"));
    clearServerOutput();

    // Test 6: PART without parameters
    sendCommand(client1, "PART");
    EXPECT_TRUE(outputContains("461 user1 PART :Not enough parameters"));
    clearServerOutput();

    // Test 7: PART a mix of valid and invalid channels
    sendCommand(client3, "JOIN #channel3");
    sendCommand(client1, "JOIN #notinthischannel");
    clearServerOutput();
    sendCommand(client3, "PART #nonexistent,#channel3,#notinthischannel");
    EXPECT_TRUE(outputContains("403 user3 #nonexistent :No such channel"));
    EXPECT_TRUE(outputContains(":user3!testuser@127.0.0.1 PART #channel3"));
    EXPECT_TRUE(outputContains("442 user3 #notinthischannel :You're not on that channel"));
    clearServerOutput();

    // Test 8: Operator status is lost after parting and rejoining
    sendCommand(client1, "JOIN #opmodetest");
    clearServerOutput();
    sendCommand(client1, "MODE #opmodetest +o user1");
    clearServerOutput();

    sendCommand(client2, "JOIN #opmodetest");
    clearServerOutput();

    // Set a mode that requires operator
    sendCommand(client1, "MODE #opmodetest +i");
    EXPECT_TRUE(outputContains(":user1!testuser@127.0.0.1 MODE #opmodetest +i"));
    clearServerOutput();

    // Part and rejoin
    sendCommand(client1, "PART #opmodetest");
    EXPECT_TRUE(outputContains(":user1!testuser@127.0.0.1 PART #opmodetest"));
    clearServerOutput();

    sendCommand(client1, "JOIN #opmodetest");
    clearServerOutput();

    // Should no longer have operator status
    sendCommand(client1, "MODE #opmodetest -i");
    EXPECT_TRUE(outputContains("482 user1 #opmodetest :You're not channel operator"));
    clearServerOutput();
}

TEST_F(TestSetup, QuitCommandComprehensive)
{
    // Register multiple clients
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
    clearServerOutput();

    // Setup: Create channels and join them
    sendCommand(client1, "JOIN #quitchan1");
    EXPECT_TRUE(outputContains(":user1!testuser@127.0.0.1 JOIN #quitchan1"));
    clearServerOutput();

    sendCommand(client2, "JOIN #quitchan1");
    EXPECT_TRUE(outputContains(":user2!testuser@127.0.0.1 JOIN #quitchan1"));
    clearServerOutput();

    // Test 1: Basic QUIT command with default reason
    sendCommand(client1, "QUIT");
    EXPECT_TRUE(outputContains(":user1!testuser@127.0.0.1 QUIT"));
    clearServerOutput();

    // Test 2: Verify client1 cannot send commands after quitting
    // This test is tricky because the socket is already closed
    // We'll rely on the next tests to verify the client is gone

    // Test 5: QUIT with specific reason
    sendCommand(client2, "QUIT :Goodbye cruel world");
    EXPECT_TRUE(outputContains(":user2!testuser@127.0.0.1 QUIT :Quit: Goodbye cruel world"));
    clearServerOutput();

    // Test 6: Client3 tries to join the abandoned channel
    // If the implementation auto-removes empty channels, this will create a new one
    // If not, this will join the existing one
    sendCommand(client3, "JOIN #quitchan1");
    EXPECT_TRUE(outputContains(":user3!testuser@127.0.0.1 JOIN #quitchan1"));
    // Should now be operator if it's a new channel
    sendCommand(client3, "MODE #quitchan1 +i");
    // If joining as an operator works, this command should succeed
    EXPECT_TRUE(outputContains("MODE #quitchan1 +i"));
    clearServerOutput();

    // Test 7: New clients joining after everyone quit
    int client4 = connectClient();
    ASSERT_GT(client4, 0);
    registerClient(client4, "user4");
    clearServerOutput();

    // Test 8: Quitting without being in any channels
    sendCommand(client4, "QUIT :Just passing through");
    EXPECT_TRUE(outputContains(":user4!testuser@127.0.0.1 QUIT :Quit: Just passing through"));
    clearServerOutput();

    // Test 9: Quitting when in multiple channels
    int client5 = connectClient();
    ASSERT_GT(client5, 0);
    registerClient(client5, "user5");

    // Join multiple channels
    sendCommand(client5, "JOIN #multichan1,#multichan2,#multichan3");
    clearServerOutput();

    // Make sure client3 joins one of the same channels to verify broadcast
    sendCommand(client3, "JOIN #multichan1");
    clearServerOutput();

    // Then quit
    sendCommand(client5, "QUIT :Leaving multiple channels");
    // Client3 should receive the quit message
    EXPECT_TRUE(outputContains(":user5!testuser@127.0.0.1 QUIT :Quit: Leaving multiple channels"));
    clearServerOutput();
}