#include <gtest/gtest.h>
#include <ChannelManager.hpp>
#include <Client.hpp>
#include <Channel.hpp>

class ChannelManagerTest : public ::testing::Test
{
protected:
    ChannelManager channelManager;
    Client *creator;
    Client *regularUser;

    void SetUp() override
    {
        creator = new Client(10);
        creator->setNickname("creator");
        regularUser = new Client(11);
        regularUser->setNickname("regular");
    }

    void TearDown() override
    {
        delete creator;
        delete regularUser;
    }
};

// Test basic channel creation and existence
TEST_F(ChannelManagerTest, CreationAndExistence)
{
    // Initially no channel exists
    EXPECT_FALSE(channelManager.channelExists("#test"));

    // Create a channel
    channelManager.createChannel("#test", *creator);

    // Check if channel exists
    EXPECT_TRUE(channelManager.channelExists("#test"));
}

// Test channel removal
TEST_F(ChannelManagerTest, RemoveChannel)
{
    // Create and verify channel
    channelManager.createChannel("#test", *creator);
    EXPECT_TRUE(channelManager.channelExists("#test"));

    // Remove and verify it's gone
    channelManager.removeChannel("#test");
    EXPECT_FALSE(channelManager.channelExists("#test"));
}

// Test case mapping for channel names
TEST_F(ChannelManagerTest, CaseMappingTest)
{
    // Create with one case, check with another
    channelManager.createChannel("#TestChannel", *creator);

    // These should all find the same channel
    EXPECT_TRUE(channelManager.channelExists("#TestChannel"));
    EXPECT_TRUE(channelManager.channelExists("#testchannel"));
    EXPECT_TRUE(channelManager.channelExists("#TESTCHANNEL"));
    EXPECT_TRUE(channelManager.channelExists("#tEsTcHaNnEl"));

    // Get the channel using different cases
    Channel &channel1 = channelManager.getChannel("#TestChannel");
    Channel &channel2 = channelManager.getChannel("#testchannel");

    // They should be the same object
    EXPECT_EQ(&channel1, &channel2);
    EXPECT_EQ(channel1.getName(), channel2.getName());
}

// Test channel access using getChannel
TEST_F(ChannelManagerTest, GetChannel)
{
    // Create a channel
    channelManager.createChannel("#test", *creator);

    // Get the channel
    Channel &channel = channelManager.getChannel("#test");

    // Check properties
    EXPECT_EQ(channel.getName(), "#test");
    EXPECT_TRUE(creator->isOnChannel(&channel)); // Creator should be in the channel
}

// Test exception when getting non-existent channel
TEST_F(ChannelManagerTest, GetNonExistentChannel)
{
    // Try to get a channel that doesn't exist
    EXPECT_THROW(channelManager.getChannel("#nonexistent"), std::out_of_range);
}

// Test multiple channel handling
TEST_F(ChannelManagerTest, MultipleChannels)
{
    // Create multiple channels
    channelManager.createChannel("#first", *creator);
    channelManager.createChannel("#second", *regularUser);
    channelManager.createChannel("#third", *creator);

    // Check they all exist
    EXPECT_TRUE(channelManager.channelExists("#first"));
    EXPECT_TRUE(channelManager.channelExists("#second"));
    EXPECT_TRUE(channelManager.channelExists("#third"));

    // Remove one
    channelManager.removeChannel("#second");
    EXPECT_FALSE(channelManager.channelExists("#second"));
    EXPECT_TRUE(channelManager.channelExists("#first"));
    EXPECT_TRUE(channelManager.channelExists("#third"));
}

// Test attempt to create duplicate channel
TEST_F(ChannelManagerTest, CreateDuplicateChannel)
{
    // Create a channel
    channelManager.createChannel("#test", *creator);

    // Attempt to create the same channel again (should fail gracefully)
    channelManager.createChannel("#test", *regularUser);

    // Attempt to create the same channel again Casemapped differently
    channelManager.createChannel("#TEST", *regularUser);

    // Original creator should still be the channel's creator
    Channel &channel = channelManager.getChannel("#test");
    EXPECT_TRUE(creator->isOnChannel(&channel));
}
