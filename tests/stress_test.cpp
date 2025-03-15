#include <gtest/gtest.h>
#include <ClientIndex.hpp>
#include <Server.hpp>
#include <thread>
#include <vector>
#include <mutex>
#include <atomic>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <cstring>
#include <chrono>
#include <common.hpp>
#include <sys/resource.h>

class StressTest : public ::testing::Test
{
protected:
    Server *server;
    std::thread serverThread;
    std::atomic<bool> serverRunning{false};

    // For capturing stdout
    std::stringstream capturedOutput;
    std::streambuf *originalCoutBuffer;

    void SetUp() override
    {
        // Redirect stdout to our stringstream
        originalCoutBuffer = std::cout.rdbuf();
        std::cout.rdbuf(capturedOutput.rdbuf());

        server = new Server();
        serverRunning = true;
        serverThread = std::thread([this]() {
            try {
                this->server->start("42");
            }
            catch (const std::exception &e) {
                std::cerr << "Server exception: " << e.what() << std::endl;
            }
        });

        // Give the server time to initialize
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }

    void TearDown() override
    {
        if (serverRunning) {
            server->shutdown();
            serverRunning = false;
        }
        if (serverThread.joinable()) {
            serverThread.join();
        }
        delete server;

        // Restore stdout
        std::cout.rdbuf(originalCoutBuffer);
    }

    // Check if a specific pattern appears in the server logs
    bool serverLogContains(const std::string &pattern)
    {
        return capturedOutput.str().find(pattern) != std::string::npos;
    }

    // Count occurrences of a pattern in logs
    int countInServerLog(const std::string &pattern)
    {
        std::string logs = capturedOutput.str();
        int count = 0;
        size_t pos = 0;
        while ((pos = logs.find(pattern, pos)) != std::string::npos) {
            ++count;
            pos += pattern.length();
        }
        return count;
    }

    void printFdLimits()
    {
        struct rlimit rlim;
        if (getrlimit(RLIMIT_NOFILE, &rlim) == 0) {
            std::cout << "File descriptor limits: soft=" << rlim.rlim_cur
                      << ", hard=" << rlim.rlim_max << std::endl;
        }
        else {
            std::cerr << "Failed to get file descriptor limits: " << strerror(errno) << std::endl;
        }
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

        return sockfd;
    }

    // Helper function to send IRC commands
    bool sendCommand(int sockfd, const std::string &cmd)
    {
        std::string command = cmd + "\r\n";
        return send(sockfd, command.c_str(), command.length(), 0) > 0;
    }

    // Helper function to read response
    std::string readResponse(int sockfd, int timeout_ms = 1000)
    {
        char buffer[512];
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
};

TEST_F(StressTest, MassClientConnections)
{
    const int NUM_CLIENTS = 25;
    std::vector<std::thread> clientThreads;
    std::vector<int> clientFds(NUM_CLIENTS, -1);
    std::mutex mtx;
    std::atomic<int> connected{0};
    std::atomic<int> registered{0};

    // Create client threads
    for (int i = 0; i < NUM_CLIENTS; i++) {
        clientThreads.emplace_back([i, &clientFds, &mtx, &connected, &registered, this]() {
            // Sleep a bit to stagger connections
            std::this_thread::sleep_for(std::chrono::milliseconds(i * 10));

            // Connect client
            int sockfd = this->connectClient();
            if (sockfd < 0) {
                std::cerr << "Client " << i << " failed to connect" << std::endl;
                return;
            }

            {
                std::lock_guard<std::mutex> lock(mtx);
                clientFds[i] = sockfd;
            }

            connected++;

            // Read welcome message
            // std::string response = this->readResponse(sockfd);

            // Register user
            std::string nick = "user" + std::to_string(i);
            this->sendCommand(sockfd, "PASS 42");
            this->sendCommand(sockfd, "NICK " + nick);
            this->sendCommand(sockfd, "USER " + nick + " 0 * :Test User " + std::to_string(i));

            // Give server time to process registration
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            registered++;

            // Wait for all clients to connect before proceeding
            while (registered < NUM_CLIENTS) {
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }

            // Wait another second
            // std::this_thread::sleep_for(std::chrono::seconds(1));

            // Quit
            this->sendCommand(sockfd, "QUIT :Leaving");
            std::this_thread::sleep_for(std::chrono::milliseconds(20));
            close(sockfd);
        });
    }

    // Wait for all client threads to finish
    for (auto &t : clientThreads) {
        if (t.joinable()) {
            t.join();
        }
    }

    // Give server time to process all disconnections
    std::this_thread::sleep_for(std::chrono::milliseconds(10));

    // Get the server log contents
    std::string serverOutput = capturedOutput.str();
    std::cerr << serverOutput;

    // 1. Count WELCOME messages
    int welcomeCount = countInServerLog("Welcome to J-A-S Network");
    EXPECT_GE(welcomeCount, NUM_CLIENTS);

    // 3. Count QUIT messages
    int quitCount = countInServerLog("QUIT");
    EXPECT_GE(quitCount, NUM_CLIENTS);

    // 4. Verify final client count is zero
    EXPECT_EQ(server->getClients().size(), 0);

    // 5. Check that all connections were successful
    EXPECT_EQ(connected.load(), NUM_CLIENTS);
    EXPECT_EQ(registered.load(), NUM_CLIENTS);

    // Close any remaining open sockets
    for (int fd : clientFds) {
        if (fd != -1) {
            close(fd);
        }
    }
}
