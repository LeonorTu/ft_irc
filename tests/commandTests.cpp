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

class CommandTest : public ::testing::Test
{
protected:
    Server *server;
    std::thread serverThread;
    int clientFd1;
    int clientFd2;
    std::atomic<bool> serverRunning{false};

    // For capturing stdout
    std::stringstream capturedOutput;
    std::streambuf *originalCoutBuffer;

    // Sync mechanisms
    std::mutex outputMutex;
    std::condition_variable serverStartedCv;
    bool serverStarted = false;

    void SetUp() override
    {
        // Capture stdout
        originalCoutBuffer = std::cout.rdbuf();
        std::cout.rdbuf(capturedOutput.rdbuf());

        // Start the server in a separate thread
        server = new Server();
        serverThread = std::thread([this]() {
            {
                std::unique_lock<std::mutex> lock(outputMutex);
                serverRunning = true;
                serverStarted = true;
                serverStartedCv.notify_one();
            }
            this->server->start("42"); // Use "42" as the password
        });

        // Wait for the server to start
        {
            std::unique_lock<std::mutex> lock(outputMutex);
            serverStartedCv.wait(lock, [this] { return serverStarted; });
        }

        // Give the server time to initialize
        std::this_thread::sleep_for(std::chrono::milliseconds(200));

        // Connect two client sockets
        clientFd1 = connectClient();
        ASSERT_GT(clientFd1, 0) << "Failed to connect first client";

        clientFd2 = connectClient();
        ASSERT_GT(clientFd2, 0) << "Failed to connect second client";

        // Clear any initial server output
        clearOutput();
    }

    void TearDown() override
    {
        // Close client connections
        if (clientFd1 > 0)
            close(clientFd1);
        if (clientFd2 > 0)
            close(clientFd2);

        // Shutdown the server
        serverRunning = false;
        server->shutdown();
        if (serverThread.joinable()) {
            serverThread.join();
        }

        delete server;

        // Restore stdout
        std::cout.rdbuf(originalCoutBuffer);

        // For debugging, uncomment to see captured output
        std::cerr << "Test output: " << capturedOutput.str() << std::endl;
    }

    // Helper function to connect a client
    int connectClient()
    {
        int sockfd = socket(AF_INET, SOCK_STREAM, 0);
        if (sockfd < 0) {
            return -1;
        }

        struct sockaddr_in serv_addr;
        std::memset(&serv_addr, 0, sizeof(serv_addr));
        serv_addr.sin_family = AF_INET;
        serv_addr.sin_port = htons(SERVER_PORT);
        inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr);

        if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
            close(sockfd);
            return -1;
        }

        // Set non-blocking
        int flags = fcntl(sockfd, F_GETFL, 0);
        fcntl(sockfd, F_SETFL, flags | O_NONBLOCK);

        return sockfd;
    }

    // Helper to send an IRC command
    void sendCommand(int sockfd, const std::string &cmd)
    {
        std::string command = cmd + "\r\n";
        send(sockfd, command.c_str(), command.length(), 0);
    }

    // Helper to read response (with timeout)
    std::string readResponse(int sockfd, int timeout_ms = 1000)
    {
        char buffer[1024];
        std::string response;

        fd_set readfds;
        struct timeval tv;
        tv.tv_sec = timeout_ms / 1000;
        tv.tv_usec = (timeout_ms % 1000) * 1000;

        FD_ZERO(&readfds);
        FD_SET(sockfd, &readfds);

        if (select(sockfd + 1, &readfds, NULL, NULL, &tv) > 0) {
            int bytes = recv(sockfd, buffer, sizeof(buffer) - 1, 0);
            if (bytes > 0) {
                buffer[bytes] = '\0';
                response = buffer;
            }
        }

        return response;
    }

    // Register a client with specified nickname
    void registerClient(int sockfd, const std::string &nickname)
    {
        sendCommand(sockfd, "PASS 42");
        sendCommand(sockfd, "NICK " + nickname);
        sendCommand(sockfd, "USER " + nickname + " 0 * :Real " + nickname);
    }
    // Check if output contains a specific string
    bool outputContains(const std::string &text)
    {
        std::string out = capturedOutput.str();
        return out.find(text) != std::string::npos;
    }

    // Clear the captured output buffer
    void clearOutput()
    {
        std::cerr << capturedOutput.str() << std::endl;
        capturedOutput.str("");
        capturedOutput.clear();
    }

    // Output current buffer (for debugging)
    void dumpOutput()
    {
        std::cerr << "=== OUTPUT DUMP ===\n" << capturedOutput.str() << "\n=== END DUMP ===\n";
    }
};

// Test successful nickname change
TEST_F(CommandTest, NickCommandSuccess)
{
    // Register first client
    registerClient(clientFd1, "user1");

    // Change nickname
    sendCommand(clientFd1, "NICK newname");

    // Wait for response
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    // Check if nickname was changed successfully
    EXPECT_TRUE(outputContains(":user1 NICK newname"));
}

// Test nickname collision
TEST_F(CommandTest, NickCommandCollision)
{
    // Register both clients with different nicknames
    registerClient(clientFd1, "user1");
    registerClient(clientFd2, "user2");
    // Try to change second client to first client's nickname
    sendCommand(clientFd2, "NICK user1");

    // Wait for response
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    // Check if error message was sent
    EXPECT_TRUE(outputContains("433 user2 user1 :Nickname is already in use"));
}

// Test invalid nickname
TEST_F(CommandTest, NickCommandInvalidName)
{
    // Register client
    registerClient(clientFd1, "user1");

    // Try to change to invalid nickname (starts with number)
    sendCommand(clientFd1, "NICK 1invalid");

    // Wait for response
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    // Check if error message was sent
    EXPECT_TRUE(outputContains("432 user1 1invalid"));
}

// Test nickname too long
TEST_F(CommandTest, NickCommandTooLong)
{
    // Register client
    registerClient(clientFd1, "user1");

    // Generate a nickname that exceeds NICKLEN
    std::string longNick(NICKLEN + 5, 'a');

    // Try to change to long nickname
    sendCommand(clientFd1, "NICK " + longNick);

    // Wait for response
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    // Either it gets truncated or rejected - verify current nickname isn't the full long one
    EXPECT_FALSE(outputContains(":" + longNick));

    // Check that either an error was sent or a truncated version was accepted
    bool success = outputContains("NICK " + longNick.substr(0, NICKLEN)) ||
                   outputContains("432 user1 " + longNick);
    EXPECT_TRUE(success);
}
