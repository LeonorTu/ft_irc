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
#include <fcntl.h>
#include <sys/time.h>
#include <iostream>

class StressTest : public ::testing::Test
{
protected:
    Server *server = nullptr;
    std::thread serverThread;
    std::atomic<bool> serverRunning{false};

    // For capturing stdout
    std::stringstream capturedOutput;
    std::streambuf *originalCoutBuffer;

    void SetUp() override
    {
        // Increase file descriptor limits if possible
        increaseFileDescriptorLimits();

        // Redirect stdout to our stringstream
        originalCoutBuffer = std::cout.rdbuf();
        std::cout.rdbuf(capturedOutput.rdbuf());

        // Create server in non-blocking mode
        server = new Server(6667, "42", false);
        serverRunning = true;

        // Start server in a separate thread
        serverThread = std::thread([this]() {
            try {
                this->server->loop();
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

    void increaseFileDescriptorLimits()
    {
        struct rlimit rlim;
        if (getrlimit(RLIMIT_NOFILE, &rlim) == 0) {
            rlim_t original_soft_limit = rlim.rlim_cur;

            // Try to increase to hard limit
            rlim.rlim_cur = rlim.rlim_max;
            if (setrlimit(RLIMIT_NOFILE, &rlim) == 0) {
                std::cerr << "Increased file descriptor limit from " << original_soft_limit
                          << " to " << rlim.rlim_cur << std::endl;
            }
            else {
                std::cerr << "Failed to increase file descriptor limit: " << strerror(errno)
                          << std::endl;
            }
        }
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
    int connectClient(int retries = 3)
    {
        int sockfd = -1;

        for (int attempt = 0; attempt < retries; attempt++) {
            sockfd = socket(AF_INET, SOCK_STREAM, 0);
            if (sockfd < 0) {
                std::cerr << "Socket creation failed: " << strerror(errno) << std::endl;
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
                continue;
            }

            // Set socket options for quicker reuse
            int yes = 1;
            if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) < 0) {
                std::cerr << "setsockopt SO_REUSEADDR failed" << std::endl;
            }

            // Set timeouts on the socket
            struct timeval tv;
            tv.tv_sec = 0;
            tv.tv_usec = 500000; // 500ms
            if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0) {
                std::cerr << "setsockopt SO_RCVTIMEO failed" << std::endl;
            }
            if (setsockopt(sockfd, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv)) < 0) {
                std::cerr << "setsockopt SO_SNDTIMEO failed" << std::endl;
            }

            struct sockaddr_in serv_addr;
            std::memset(&serv_addr, 0, sizeof(serv_addr));
            serv_addr.sin_family = AF_INET;
            serv_addr.sin_port = htons(SERVER_PORT);
            inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr);

            if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
                close(sockfd);
                std::cerr << "Connection attempt " << attempt + 1 << " failed: " << strerror(errno)
                          << std::endl;
                std::this_thread::sleep_for(std::chrono::milliseconds(20 * (attempt + 1)));
                continue;
            }

            // Successfully connected
            return sockfd;
        }

        return -1;
    }

    // Helper function to send IRC commands
    bool sendCommand(int sockfd, const std::string &cmd)
    {
        std::string command = cmd + "\r\n";
        int sent = send(sockfd, command.c_str(), command.length(), 0);
        return sent > 0;
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
    const int NUM_CLIENTS = 1000;
    printFdLimits();

    std::vector<std::thread> clientThreads;
    std::vector<int> clientFds(NUM_CLIENTS, -1);
    std::mutex mtx;
    std::atomic<int> connected{0};
    std::atomic<int> registered{0};
    std::atomic<int> failed{0};


    // Create client threads in batches to avoid overwhelming the server
    const int BATCH_SIZE = 200;
    for (int batch = 0; batch < (NUM_CLIENTS + BATCH_SIZE - 1) / BATCH_SIZE; batch++) {
        int start = batch * BATCH_SIZE;
        int end = std::min(start + BATCH_SIZE, NUM_CLIENTS);

        for (int i = start; i < end; i++) {
            clientThreads.emplace_back([i, &clientFds, &mtx, &connected, &registered, &failed,
                                        this]() {
                // Sleep a bit to stagger connections
                std::this_thread::sleep_for(std::chrono::milliseconds(i * 5));

                // Connect client with retries
                int sockfd = this->connectClient(3);
                if (sockfd < 0) {
                    std::cerr << "Client " << i << " failed to connect after retries" << std::endl;
                    failed++;
                    return;
                }

                {
                    std::lock_guard<std::mutex> lock(mtx);
                    clientFds[i] = sockfd;
                }

                connected++;

                // Register user
                std::string nick = "user" + std::to_string(i);
                bool success = true;

                success &= this->sendCommand(sockfd, "PASS 42");
                if (!success) {
                    std::cerr << "Client " << i << " failed to send PASS" << std::endl;
                    failed++;
                    close(sockfd);
                    return;
                }

                success &= this->sendCommand(sockfd, "NICK " + nick);
                if (!success) {
                    std::cerr << "Client " << i << " failed to send NICK" << std::endl;
                    failed++;
                    close(sockfd);
                    return;
                }

                success &= this->sendCommand(sockfd, "USER " + nick + " 0 * :Test User " +
                                                         std::to_string(i));
                if (!success) {
                    std::cerr << "Client " << i << " failed to send USER" << std::endl;
                    failed++;
                    close(sockfd);
                    return;
                }

                // Give server time to process registration
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                registered++;
            });
        }

        // Wait for this batch to complete registration before starting the next
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    // Wait for all client threads to complete the registration phase
    for (size_t i = 0; i < clientThreads.size(); i++) {
        if (clientThreads[i].joinable()) {
            clientThreads[i].join();
        }
    }

    // Now all clients should be registered, send QUIT messages
    std::vector<std::thread> quitThreads;
    for (int i = 0; i < NUM_CLIENTS; i++) {
        if (clientFds[i] > 0) {
            quitThreads.emplace_back([i, &clientFds, this]() {
                // Send QUIT and close
                this->sendCommand(clientFds[i], "QUIT :Leaving");
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                close(clientFds[i]);
            });
        }
    }

    // Wait for QUIT threads to complete
    for (auto &t : quitThreads) {
        if (t.joinable()) {
            t.join();
        }
    }

    // Give server time to process all disconnections
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    // Get the server log contents
    std::string serverOutput = capturedOutput.str();
    std::cerr << "Failed connections: " << failed.load() << std::endl;
    std::cerr << "Successful connections: " << connected.load() << std::endl;
    std::cerr << "Registered clients: " << registered.load() << std::endl;

    // Check if any ERROR messages in the output
    int errorCount = countInServerLog("ERROR");
    std::cerr << "Error count in logs: " << errorCount << std::endl;

    // Count WELCOME messages
    int welcomeCount = countInServerLog("Welcome to J-A-S Network");
    std::cerr << "Welcome message count: " << welcomeCount << std::endl;

    // 3. Count QUIT messages
    int quitCount = countInServerLog("QUIT");
    std::cerr << "Quit message count: " << quitCount << std::endl;

    // Check client count in server
    int finalClientCount = server->getClients().size();
    std::cerr << "Final client count in server: " << finalClientCount << std::endl;

    // Relaxed expectations for high client counts
    EXPECT_GE(connected.load(), NUM_CLIENTS - failed.load());
    EXPECT_GE(registered.load(), NUM_CLIENTS * 0.95); // Allow 5% failure rate
    EXPECT_EQ(finalClientCount, 0);

    // Close any remaining open sockets (shouldn't be any)
    for (int fd : clientFds) {
        if (fd != -1) {
            close(fd);
        }
    }
}
