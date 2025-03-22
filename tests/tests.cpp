#include <gtest/gtest.h>
#include <Server.hpp>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <thread>
#include <chrono>
class ServerTest : public ::testing::Test
{
protected:
    Server *server = nullptr;
    void SetUp() override
    {
        // Runs before each test
    }

    void TearDown() override
    {
        // Runs after each test
        if (server) {
            server->shutdown();
            delete server;
            server = nullptr;
        }
    }
};

// Existing test
TEST_F(ServerTest, InitializationTest)
{
    // Create server in non-blocking mode
    server = new Server(6667, "42", false);
    ASSERT_NE(server, nullptr);
    EXPECT_EQ(server->getPassword(), "42");
}

TEST_F(ServerTest, StartStopTest)
{
    // Create server in non-blocking mode
    server = new Server(6667, "42", false);
    ASSERT_NE(server, nullptr);
    EXPECT_FALSE(server->getIsPaused());
}

TEST_F(ServerTest, WelcomeMessageTest)
{
    // Create server in non-blocking mode
    server = new Server(6667, "42", false);
    ASSERT_NE(server, nullptr);

    // Start the server in a separate thread
    std::thread server_thread([this]() { this->server->loop(); });
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    // Use a short timeout to close automatically
    FILE *nc = popen(
        "printf 'PASS 42\\r\\nNICK Bob\\r\\nUSER Bob 0 * :Realname\\r\\n' | nc -w 1 localhost 6667",
        "r");
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
    EXPECT_NE(output.find("005"), std::string::npos);

    pclose(nc);

    this->server->shutdown();
    server_thread.join();
}

TEST_F(ServerTest, PauseResumeTest)
{
    // Create server in non-blocking mode
    server = new Server(6667, "42", false);
    ASSERT_NE(server, nullptr);

    EXPECT_FALSE(server->getIsPaused());
    server->pause();
    EXPECT_TRUE(server->getIsPaused());
    server->resume();
    EXPECT_FALSE(server->getIsPaused());
}

TEST_F(ServerTest, MessageSizeLimitTest)
{
    // Create server in non-blocking mode
    server = new Server(6667, "42", false);
    ASSERT_NE(server, nullptr);

    // Start the server in a separate thread
    std::thread server_thread([this]() { this->server->loop(); });
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    // Create an oversized message
    std::string large_message(600, 'A');

    FILE *nc = popen(("echo '" + large_message + "' | nc localhost 6667").c_str(), "r");
    ASSERT_NE(nc, nullptr);
    pclose(nc);

    this->server->shutdown();
    server_thread.join();
}
