#include <ChannelManager.hpp>
#include "TestSetup.hpp"

TEST_F(TestSetup, TestPrivmsgNonExistentUser)
{
    int client1 = connectClient();
    ASSERT_GT(client1, 0);
    registerClient(client1, "user1");
    clearServerOutput();

    sendCommand(client1, "PRIVMSG nonExistentUser :Hello");
    EXPECT_TRUE(outputContains("401 user1 nonExistentUser :No such nick/channel"));
}

TEST_F(TestSetup, TestPrivmsgToSelf)
{
    int client1 = connectClient();
    ASSERT_GT(client1, 0);
    registerClient(client1, "user1");
    clearServerOutput();

    sendCommand(client1, "PRIVMSG user1 :Hello myself");
    EXPECT_TRUE(outputContains(":user1!testuser@127.0.0.1 PRIVMSG user1 :Hello myself"));
}

TEST_F(TestSetup, TestPrivmsgNonExistentChannel)
{
    int client1 = connectClient();
    ASSERT_GT(client1, 0);
    registerClient(client1, "user1");
    clearServerOutput();

    sendCommand(client1, "PRIVMSG #nonExistentChannel :Hello");
    EXPECT_TRUE(outputContains("403 user1 #nonExistentChannel :No such channel"));
}

TEST_F(TestSetup, TestPrivmsgWithSpecialChars)
{
    int client1 = connectClient();
    int client2 = connectClient();
    ASSERT_GT(client1, 0);
    ASSERT_GT(client2, 0);
    registerClient(client1, "user1");
    registerClient(client2, "user2");
    sendCommand(client1, "JOIN #test");
    sendCommand(client2, "JOIN #test");
    clearServerOutput();

    sendCommand(client1, "PRIVMSG #test :Hello, world! @#$%^&*()");
    EXPECT_TRUE(outputContains(":user1!testuser@127.0.0.1 PRIVMSG #test :Hello, world! @#$%^&*()"));
}

TEST_F(TestSetup, TestPrivmsgMaxLength)
{
    int client1 = connectClient();
    int client2 = connectClient();
    ASSERT_GT(client1, 0);
    ASSERT_GT(client2, 0);
    registerClient(client1, "user1");
    registerClient(client2, "user2");
    sendCommand(client1, "JOIN #test");
    sendCommand(client2, "JOIN #test");
    clearServerOutput();

    std::string longMessage(299, 'a');
    sendCommand(client1, "PRIVMSG #test :" + longMessage);
    EXPECT_TRUE(outputContains(":user1!testuser@127.0.0.1 PRIVMSG #test :" + longMessage));
}

TEST_F(TestSetup, TestPrivmsgEmptyContent)
{
    int client1 = connectClient();
    int client2 = connectClient();
    ASSERT_GT(client1, 0);
    ASSERT_GT(client2, 0);
    registerClient(client1, "user1");
    registerClient(client2, "user2");
    sendCommand(client1, "JOIN #test");
    sendCommand(client2, "JOIN #test");
    clearServerOutput();

    sendCommand(client1, "PRIVMSG #test :");
    // EXPECT_TRUE(outputContains(":user1!testuser@127.0.0.1 PRIVMSG #test :"));
    EXPECT_TRUE(outputContains("412 user1 :No text to send"));
}

TEST_F(TestSetup, TestPrivmsgNoTarget)
{
    int client1 = connectClient();
    ASSERT_GT(client1, 0);
    registerClient(client1, "user1");
    clearServerOutput();

    sendCommand(client1, "PRIVMSG");
    EXPECT_TRUE(outputContains("461 user1 PRIVMSG :Not enough parameters"));
}

TEST_F(TestSetup, TestPrivmsgNoMessage)
{
    int client1 = connectClient();
    ASSERT_GT(client1, 0);
    registerClient(client1, "user1");
    sendCommand(client1, "JOIN #test");
    clearServerOutput();

    sendCommand(client1, "PRIVMSG #test");
    EXPECT_TRUE(outputContains("461 user1 PRIVMSG :Not enough parameters"));
}

TEST_F(TestSetup, TestPrivmsgWithoutColon)
{
    int client1 = connectClient();
    int client2 = connectClient();
    ASSERT_GT(client1, 0);
    ASSERT_GT(client2, 0);
    registerClient(client1, "user1");
    registerClient(client2, "user2");
    sendCommand(client1, "JOIN #test");
    sendCommand(client2, "JOIN #test");
    clearServerOutput();

    sendCommand(client1, "PRIVMSG #test Hello without colon");
    EXPECT_TRUE(outputContains(":user1!testuser@127.0.0.1 PRIVMSG #test :Hello"));
}

TEST_F(TestSetup, TestPrivmsgInvalidTarget)
{
    int client1 = connectClient();
    ASSERT_GT(client1, 0);
    registerClient(client1, "user1");
    clearServerOutput();

    sendCommand(client1, "PRIVMSG user@1 :Hello");
    EXPECT_TRUE(outputContains("432 user1 user@1 :Erroneus nickname"));
}

TEST_F(TestSetup, TestNoticeToChannel)
{
    int client1 = connectClient();
    int client2 = connectClient();
    ASSERT_GT(client1, 0);
    ASSERT_GT(client2, 0);
    registerClient(client1, "user1");
    registerClient(client2, "user2");
    sendCommand(client1, "JOIN #test");
    sendCommand(client2, "JOIN #test");
    clearServerOutput();

    sendCommand(client1, "NOTICE #test :This is a notice message");
    EXPECT_TRUE(outputContains(":user1!testuser@127.0.0.1 NOTICE #test :This is a notice message"));
}

TEST_F(TestSetup, TestNoticeToUser)
{
    int client1 = connectClient();
    int client2 = connectClient();
    ASSERT_GT(client1, 0);
    ASSERT_GT(client2, 0);
    registerClient(client1, "user1");
    registerClient(client2, "user2");
    clearServerOutput();

    sendCommand(client1, "NOTICE user2 :This is a direct notice");
    EXPECT_TRUE(outputContains(":user1!testuser@127.0.0.1 NOTICE user2 :This is a direct notice"));
}

TEST_F(TestSetup, TestNoticeToNonExistentTarget)
{
    int client1 = connectClient();
    ASSERT_GT(client1, 0);
    registerClient(client1, "user1");
    clearServerOutput();

    sendCommand(client1, "NOTICE nonexistentuser :Notices should not error");
    EXPECT_FALSE(outputContains("401"));
}

TEST_F(TestSetup, TestPrivmsgToAllChannelMembers)
{
    int client1 = connectClient();
    int client2 = connectClient();
    int client3 = connectClient();
    ASSERT_GT(client1, 0);
    ASSERT_GT(client2, 0);
    ASSERT_GT(client3, 0);
    registerClient(client1, "user1");
    registerClient(client2, "user2");
    registerClient(client3, "user3");
    sendCommand(client1, "JOIN #test");
    sendCommand(client2, "JOIN #test");
    sendCommand(client3, "JOIN #test");
    clearServerOutput();

    sendCommand(client1, "PRIVMSG #test :Message to all channel members");
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    EXPECT_TRUE(
        outputContains(":user1!testuser@127.0.0.1 PRIVMSG #test :Message to all channel members"));
}

TEST_F(TestSetup, TestPrivmsgMultipleTargets)
{
    int client1 = connectClient();
    int client2 = connectClient();
    int client3 = connectClient();
    ASSERT_GT(client1, 0);
    ASSERT_GT(client2, 0);
    ASSERT_GT(client3, 0);
    registerClient(client1, "user1");
    registerClient(client2, "user2");
    registerClient(client3, "user3");
    clearServerOutput();

    sendCommand(client1, "PRIVMSG user2,nonexistent,user3 :Message to valid and invalid");
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    EXPECT_TRUE(
        outputContains(":user1!testuser@127.0.0.1 PRIVMSG user2 :Message to valid and invalid"));
    EXPECT_TRUE(outputContains("401 user1 nonexistent :No such nick/channel"));
    EXPECT_TRUE(
        outputContains(":user1!testuser@127.0.0.1 PRIVMSG user3 :Message to valid and invalid"));
}

TEST_F(TestSetup, TestPrivmsgMaxMultipleTargets)
{
    int client1 = connectClient();
    int client2 = connectClient();
    int client3 = connectClient();
    int client4 = connectClient();
    int client5 = connectClient();
    int client6 = connectClient();
    ASSERT_GT(client1, 0);
    ASSERT_GT(client2, 0);
    ASSERT_GT(client3, 0);
    ASSERT_GT(client4, 0);
    ASSERT_GT(client5, 0);
    ASSERT_GT(client6, 0);
    registerClient(client1, "user1");
    registerClient(client2, "user2");
    registerClient(client3, "user3");
    registerClient(client4, "user4");
    registerClient(client5, "user5");
    registerClient(client6, "user6");
    clearServerOutput();

    sendCommand(client1,
                "PRIVMSG user1,user2,user3,user4,user5,user6 :Message to valid and invalid");
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    EXPECT_TRUE(
        outputContains(":user1!testuser@127.0.0.1 PRIVMSG user1 :Message to valid and invalid"));
    EXPECT_TRUE(
        outputContains(":user1!testuser@127.0.0.1 PRIVMSG user2 :Message to valid and invalid"));
    EXPECT_TRUE(
        outputContains(":user1!testuser@127.0.0.1 PRIVMSG user3 :Message to valid and invalid"));
    EXPECT_TRUE(
        outputContains(":user1!testuser@127.0.0.1 PRIVMSG user4 :Message to valid and invalid"));
}

TEST_F(TestSetup, TestNoticeMultipleTargets)
{
    int client1 = connectClient();
    int client2 = connectClient();
    ASSERT_GT(client1, 0);
    ASSERT_GT(client2, 0);
    registerClient(client1, "user1");
    registerClient(client2, "user2");
    clearServerOutput();

    sendCommand(client1, "NOTICE user2,nonexistent :Notice to valid and invalid");
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    EXPECT_TRUE(
        outputContains(":user1!testuser@127.0.0.1 NOTICE user2 :Notice to valid and invalid"));
    EXPECT_FALSE(outputContains("401"));
}