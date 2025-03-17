#include <gtest/gtest.h>
#include <MessageParser.hpp>

bool VERBOSE_OUTPUT = true;

void printing(MessageParser &test)
{
    if (!VERBOSE_OUTPUT)
        return;
    std::cout << "Source : " << test.getContext().source << std::endl;
    std::cout << "Command : " << test.getCommand() << std::endl;
    int count = 0;
    for (const auto &param : test.getContext().params) {
        std::cout << "Param[" << count << "] : " << param << std::endl;
        count++;
    }
}

// Standard PRIVMSG test
TEST(ProxyParsingTest, StandardPrivmsg)
{
    std::string testString = ":dan!d@localhost PRIVMSG #chan :Hey!";
    std::cout << "TEST: " << testString << std::endl;

    MessageParser testing(1, testString);
    testing.parseCommand();

    EXPECT_EQ(testing.getContext().source, "dan!d@localhost");
    EXPECT_EQ(testing.getCommand(), "PRIVMSG");
    ASSERT_EQ(testing.getContext().params.size(), 2);
    EXPECT_EQ(testing.getContext().params[0], "#chan");
    EXPECT_EQ(testing.getContext().params[1], "Hey!");
    printing(testing);
}

// PRIVMSG with emoticon in trailing
TEST(ProxyParsingTest, PrivmsgWithEmoticon)
{
    std::string testString = ":dan!d@localhost PRIVMSG #chan ::-) hello";
    std::cout << "TEST: " << testString << std::endl;

    MessageParser testing(1, testString);
    testing.parseCommand();

    EXPECT_EQ(testing.getContext().source, "dan!d@localhost");
    EXPECT_EQ(testing.getCommand(), "PRIVMSG");
    ASSERT_EQ(testing.getContext().params.size(), 2);
    EXPECT_EQ(testing.getContext().params[0], "#chan");
    EXPECT_EQ(testing.getContext().params[1], ":-) hello");
    printing(testing);
}

// CAP command test
TEST(ProxyParsingTest, CapCommand)
{
    std::string testString = "CAP REQ :sasl message-tags foo";
    std::cout << "TEST: " << testString << std::endl;

    MessageParser testing(1, testString);
    testing.parseCommand();

    EXPECT_EQ(testing.getContext().source, "");
    EXPECT_EQ(testing.getCommand(), "CAP");
    ASSERT_EQ(testing.getContext().params.size(), 2);
    EXPECT_EQ(testing.getContext().params[0], "REQ");
    EXPECT_EQ(testing.getContext().params[1], "sasl message-tags foo");
    printing(testing);
}

// Tagged PRIVMSG (tag should be ignored)
TEST(ProxyParsingTest, TaggedPrivmsg)
{
    std::string testString = "@id=234AB :dan!d@localhost PRIVMSG #chan :Hey what's up!";
    std::cout << "TEST: " << testString << std::endl;

    MessageParser testing(1, testString);
    testing.parseCommand();

    EXPECT_EQ(testing.getContext().source, "dan!d@localhost");
    EXPECT_EQ(testing.getCommand(), "PRIVMSG");
    ASSERT_EQ(testing.getContext().params.size(), 2);
    EXPECT_EQ(testing.getContext().params[0], "#chan");
    EXPECT_EQ(testing.getContext().params[1], "Hey what's up!");
    printing(testing);
}

// Server capability listing
TEST(ProxyParsingTest, ServerCapabilityListing)
{
    std::string testString = ":irc.example.com CAP LS * :multi-prefix extended-join sasl";
    std::cout << "TEST: " << testString << std::endl;

    MessageParser testing(1, testString);
    testing.parseCommand();

    EXPECT_EQ(testing.getContext().source, "irc.example.com");
    EXPECT_EQ(testing.getCommand(), "CAP");
    ASSERT_EQ(testing.getContext().params.size(), 3);
    EXPECT_EQ(testing.getContext().params[0], "LS");
    EXPECT_EQ(testing.getContext().params[1], "*");
    EXPECT_EQ(testing.getContext().params[2], "multi-prefix extended-join sasl");
    printing(testing);
}

// Multiple tags (tags should be ignored)
TEST(ProxyParsingTest, MultipleTagsPrivmsg)
{
    std::string testString =
        "@tag1=value1;tag2=value2 :nick!user@host PRIVMSG #channel :Hello world!";
    std::cout << "TEST: " << testString << std::endl;

    MessageParser testing(1, testString);
    testing.parseCommand();

    EXPECT_EQ(testing.getContext().source, "nick!user@host");
    EXPECT_EQ(testing.getCommand(), "PRIVMSG");
    ASSERT_EQ(testing.getContext().params.size(), 2);
    EXPECT_EQ(testing.getContext().params[0], "#channel");
    EXPECT_EQ(testing.getContext().params[1], "Hello world!");
    printing(testing);
}

// Empty message
TEST(ProxyParsingEdgeCases, EmptyMessage)
{
    std::string testString = "";
    std::cout << "TEST: Empty message" << std::endl;

    MessageParser testing(1, testString);
    testing.parseCommand();

    EXPECT_EQ(testing.getContext().source, "");
    EXPECT_EQ(testing.getCommand(), "");
    EXPECT_EQ(testing.getContext().params.size(), 0);
    printing(testing);
}

// Only whitespace
TEST(ProxyParsingEdgeCases, OnlyWhitespace)
{
    std::string testString = "   ";
    std::cout << "TEST: Only whitespace" << std::endl;

    MessageParser testing(1, testString);
    testing.parseCommand();

    EXPECT_EQ(testing.getContext().source, "");
    EXPECT_EQ(testing.getCommand(), "");
    EXPECT_EQ(testing.getContext().params.size(), 0);
    printing(testing);
}

// No source, just command
TEST(ProxyParsingEdgeCases, NoSourceJustCommand)
{
    std::string testString = "PING";
    std::cout << "TEST: No source, just command" << std::endl;

    MessageParser testing(1, testString);
    testing.parseCommand();

    EXPECT_EQ(testing.getContext().source, "");
    EXPECT_EQ(testing.getCommand(), "PING");
    EXPECT_EQ(testing.getContext().params.size(), 0);
    printing(testing);
}

// No source, command with params
TEST(ProxyParsingEdgeCases, NoSourceCommandWithParams)
{
    std::string testString = "JOIN #channel";
    std::cout << "TEST: No source, command with params" << std::endl;

    MessageParser testing(1, testString);
    testing.parseCommand();

    EXPECT_EQ(testing.getContext().source, "");
    EXPECT_EQ(testing.getCommand(), "JOIN");
    ASSERT_EQ(testing.getContext().params.size(), 1);
    EXPECT_EQ(testing.getContext().params[0], "#channel");
    printing(testing);
}

// Multiple params without trailing
TEST(ProxyParsingEdgeCases, MultipleParamsWithoutTrailing)
{
    std::string testString = "MODE #channel +o nick";
    std::cout << "Multiple params without trailing" << std::endl;
    std::cout << "TEST: " << testString << std::endl;

    MessageParser testing(1, testString);
    testing.parseCommand();

    EXPECT_EQ(testing.getContext().source, "");
    EXPECT_EQ(testing.getCommand(), "MODE");
    ASSERT_EQ(testing.getContext().params.size(), 3);
    EXPECT_EQ(testing.getContext().params[0], "#channel");
    EXPECT_EQ(testing.getContext().params[1], "+o");
    EXPECT_EQ(testing.getContext().params[2], "nick");
    printing(testing);
}
