#include <gtest/gtest.h>
#include <Channel.hpp>
#include <Client.hpp>
#include <string>

class ChannelTest : public ::testing::Test
{
protected:
    Channel *channel;
    Client *creator;
    Client *regularUser;
    std::string creatorIp = "127.0.0.1";
    std::string regularIp = "127.0.0.2";

    // For capturing stdout
    std::stringstream buffer;
    std::streambuf *oldCout;

    void SetUp() override
    {
        // Redirect cout to our buffer
        oldCout = std::cout.rdbuf(buffer.rdbuf());

        // Create mock clients
        creator = new Client(10);
        creator->setNickname("creator");
        regularUser = new Client(11);
        regularUser->setNickname("regular");

        // Create a test channel
        channel = new Channel("#test", *creator);
    }

    void TearDown() override
    {
        // std::cerr << buffer.str();

        // Restore cout
        std::cout.rdbuf(oldCout);
        // print everything to cout for visal checking
        clearOutput();
        delete channel;
        delete creator;
        delete regularUser;
    }

    bool outputContains(const std::string &text)
    {
        std::string buf = buffer.str();
        return buf.find(text) != std::string::npos;
    }

    // Reset the output buffer during tests
    void clearOutput()
    {
        std::cerr << buffer.str();
        buffer.str("");
        buffer.clear();
    }
};

// Test channel creation
TEST_F(ChannelTest, CreationTest)
{
    EXPECT_EQ(channel->getName(), "#test");
    EXPECT_TRUE(channel->hasMode(ChannelMode::OP)); // Creator should be op
    EXPECT_FALSE(channel->hasMode(ChannelMode::INVITE_ONLY));
    EXPECT_FALSE(channel->isEmpty());
}

// Test joining and leaving
TEST_F(ChannelTest, JoinAndPartTest)
{
    // Join channel
    EXPECT_FALSE(regularUser->isOnChannel(channel));
    channel->join(*regularUser);
    EXPECT_TRUE(regularUser->isOnChannel(channel));

    // Part channel
    channel->part(*regularUser, "Leaving test");
    EXPECT_FALSE(regularUser->isOnChannel(channel));
}

// Test channel modes
TEST_F(ChannelTest, ModeTest)
{
    // Set invite-only mode
    channel->setMode(*creator, true, ChannelMode::INVITE_ONLY);
    EXPECT_TRUE(channel->hasMode(ChannelMode::INVITE_ONLY));

    // Set channel key
    channel->setMode(*creator, true, ChannelMode::KEY, "secret");
    EXPECT_TRUE(channel->hasMode(ChannelMode::KEY));

    // Regular user can't join invite-only channel
    channel->join(*regularUser);
    EXPECT_FALSE(regularUser->isOnChannel(channel));

    // Remove invite-only mode
    channel->setMode(*creator, false, ChannelMode::INVITE_ONLY);
    EXPECT_FALSE(channel->hasMode(ChannelMode::INVITE_ONLY));

    // Regular user can't join with wrong key
    channel->join(*regularUser, "wrong");
    EXPECT_FALSE(regularUser->isOnChannel(channel));

    // Regular user can join with correct key if invited
    // (Need to implement invite functionality first)

    // Disable invite-only mode again (already disabled)
    channel->setMode(*creator, false, ChannelMode::INVITE_ONLY);
    EXPECT_FALSE(channel->hasMode(ChannelMode::INVITE_ONLY));

    // Now regular user can join with correct key
    channel->join(*regularUser, "secret");
    EXPECT_TRUE(regularUser->isOnChannel(channel));
}

// Test topic functionality
TEST_F(ChannelTest, TopicTest)
{
    // Join as regular user
    channel->join(*regularUser);
    clearOutput(); // Clear output from join operations

    // Set topic as creator (who is op)
    std::string newTopic = "This is a test topic";
    channel->changeTopic(*creator, newTopic);

    // Check creator set topic properly
    EXPECT_TRUE(outputContains("332 regular #test :This is a test topic"));
    EXPECT_TRUE(outputContains("333 regular #test creator"));               // Topic author info
    EXPECT_TRUE(outputContains("332 creator #test :This is a test topic")); // Both users should receive update

    // Set topic as regular user
    std::string regTopic = "Regular user topic";
    channel->changeTopic(*regularUser, regTopic);

    // Check regular user could change topic (not protected yet)
    EXPECT_TRUE(outputContains("332 regular #test :Regular user topic"));
    EXPECT_TRUE(outputContains("333 regular #test regular")); // Regular is now author
    EXPECT_TRUE(outputContains("332 creator #test :Regular user topic"));

    // Enable topic protection
    channel->setMode(*creator, true, ChannelMode::PROTECTED_TOPIC);
    EXPECT_TRUE(outputContains(":creator MODE #test +t"));
    clearOutput();

    // Verify non-op user can't change topic when protected
    std::string anotherTopic = "Trying to change topic";
    channel->changeTopic(*regularUser, anotherTopic);

    // Should see error message but no topic change notification
    EXPECT_TRUE(outputContains("482 regular #test :You're not channel operator"));
    EXPECT_FALSE(outputContains("332 regular #test :Trying to change topic"));
    EXPECT_FALSE(outputContains("332 creator #test :Trying to change topic"));
    clearOutput();

    // Give regular user op status
    channel->setMode(*creator, true, ChannelMode::OP, "regular");
    EXPECT_TRUE(outputContains(":creator MODE #test +o regular"));
    clearOutput();

    // Now regular user can change topic
    std::string opChangedTopic = "Topic changed by new op";
    channel->changeTopic(*regularUser, opChangedTopic);

    // Check topic was changed successfully
    EXPECT_TRUE(outputContains("332 regular #test :Topic changed by new op"));
    EXPECT_TRUE(outputContains("333 regular #test regular")); // Regular is still author
    EXPECT_TRUE(outputContains("332 creator #test :Topic changed by new op"));
}

// Test operator functionality
TEST_F(ChannelTest, OperatorTest)
{
    channel->join(*regularUser);

    // Regular user can't set modes
    channel->setMode(*regularUser, true, ChannelMode::INVITE_ONLY);
    EXPECT_FALSE(channel->hasMode(ChannelMode::INVITE_ONLY));

    // Give regular user op status
    channel->setMode(*creator, true, ChannelMode::OP, "regular");

    // Now they can set modes
    channel->setMode(*regularUser, true, ChannelMode::INVITE_ONLY);
    EXPECT_TRUE(channel->hasMode(ChannelMode::INVITE_ONLY));

    // Remove op status
    channel->setMode(*creator, false, ChannelMode::OP, "regular");

    // Now they can't set modes again
    channel->setMode(*regularUser, false, ChannelMode::INVITE_ONLY);
    EXPECT_TRUE(channel->hasMode(ChannelMode::INVITE_ONLY)); // Mode should remain unchanged

    // self deop
    channel->setMode(*creator, true, ChannelMode::OP, "regular");
    channel->setMode(*regularUser, false, ChannelMode::OP, "regular");
    channel->setMode(*regularUser, true, ChannelMode::OP, "regular");
    channel->setMode(*regularUser, false, ChannelMode::INVITE_ONLY);
    EXPECT_TRUE(channel->hasMode(ChannelMode::INVITE_ONLY));
}

// Test quit functionality
TEST_F(ChannelTest, QuitTest)
{
    channel->join(*regularUser);
    EXPECT_TRUE(regularUser->isOnChannel(channel));

    // User quits
    channel->quit(*regularUser, "testing QUIT");
    EXPECT_FALSE(regularUser->isOnChannel(channel));

    // Channel should still exist with creator
    EXPECT_FALSE(channel->isEmpty());

    // Creator quits no QUIT message sent back because the channel is empty
    channel->quit(*creator, "Creator quitting");

    // Channel should be empty
    EXPECT_TRUE(channel->isEmpty());
}

// Test user limit
TEST_F(ChannelTest, UserLimitTest)
{
    // Set user limit to 1 (creator is already in)
    EXPECT_FALSE(channel->hasMode(ChannelMode::LIMIT));
    channel->setMode(*creator, true, ChannelMode::LIMIT, "1");
    EXPECT_TRUE(channel->hasMode(ChannelMode::LIMIT));

    // Try to join - should fail due to limit
    channel->join(*regularUser);
    EXPECT_FALSE(regularUser->isOnChannel(channel));

    // Increase limit
    channel->setMode(*creator, true, ChannelMode::LIMIT, "2");

    // Now joining should work
    channel->join(*regularUser);
    EXPECT_TRUE(regularUser->isOnChannel(channel));
}

TEST_F(ChannelTest, JoinMessageTest)
{
    // Join channel
    channel->join(*regularUser);

    // Verify join message was sent to the client
    EXPECT_TRUE(outputContains(":regular JOIN #test\r\n"));
    // Verify they received topic info
    EXPECT_TRUE(outputContains("332") || outputContains("331"));

    // Verify they received NAMES reply
    EXPECT_TRUE(outputContains("353"));
    EXPECT_TRUE(outputContains("366"));
}

TEST_F(ChannelTest, ModeMessageTest)
{
    channel->setMode(*creator, true, ChannelMode::INVITE_ONLY);

    // Check if mode change message was broadcast
    EXPECT_TRUE(outputContains("MODE #test +i"));
}

TEST_F(ChannelTest, ErrorMessageTest)
{
    // Try to join with wrong key
    channel->setMode(*creator, true, ChannelMode::KEY, "secret");
    channel->join(*regularUser, "wrong");

    // Verify error message was sent
    EXPECT_TRUE(outputContains("475"));
}
