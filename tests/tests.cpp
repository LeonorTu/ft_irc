#include <gtest/gtest.h>
#include <server.hpp>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <thread>
#include <chrono>
#include <message.hpp>

class ServerTest : public ::testing::Test
{
protected:
    Server *server; // Shared resource for all tests

    void SetUp() override
    {
        // Runs before each test
        server = new Server();
    }

    void TearDown() override
    {
        // Runs after each test
        delete server;
    }
};

// Existing test
TEST_F(ServerTest, InitializationTest)
{
    EXPECT_EQ(server->getServerFD(), -1);
}

TEST(MessageTest, messageTest)
{
    message parsedMessage("USER jtu jtu localhost :Jingyi Tu");
    EXPECT_EQ(parsedMessage.command, "USER");
    EXPECT_EQ(parsedMessage.parameters[0], "jtu");
}

TEST_F(ServerTest, StartStopTest)
{
    std::thread server_thread([this]() { this->server->start(); });
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    this->server->stop();
    server_thread.join();
    EXPECT_FALSE(this->server->paused);
}

TEST_F(ServerTest, WelcomeMessageTest)
{
    std::thread server_thread([this]() { this->server->start(); });
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    // Use a short timeout to close automatically
    FILE *nc = popen("echo '' | nc -w 1 localhost 6667", "r");
    ASSERT_NE(nc, nullptr);

    char buffer[1024] = {0};
    std::string output;
    while (fgets(buffer, sizeof(buffer), nc) != nullptr) {
        output.append(buffer);
    }
    std::cout << output << std::endl;

    // Check welcome message components
    EXPECT_NE(output.find("001"), std::string::npos);
    EXPECT_NE(output.find("002"), std::string::npos);
    EXPECT_NE(output.find("003"), std::string::npos);
    EXPECT_NE(output.find("004"), std::string::npos);

    pclose(nc);

    this->server->stop();
    server_thread.join();
}

TEST_F(ServerTest, PauseResumeTest)
{
    EXPECT_FALSE(server->paused);
    server->pause();
    EXPECT_TRUE(server->paused);
    server->resume();
    EXPECT_FALSE(server->paused);
}

TEST_F(ServerTest, MessageSizeLimitTest)
{
    std::thread server_thread([this]() { this->server->start(); });
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    // Create an oversized message
    std::string large_message(600, 'A');

    FILE *nc = popen(("echo '" + large_message + "' | nc localhost 6667").c_str(), "r");
    ASSERT_NE(nc, nullptr);
    pclose(nc);

    this->server->stop();
    server_thread.join();
}
