#pragma once

#include <gtest/gtest.h>
#include <Server.hpp>
#include <Client.hpp>
#include <thread>
#include <sstream>
#include <iostream>
#include <string>
#include <vector>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <mutex>
#include <atomic>
#include <future>
#include <condition_variable>

// This class is similar to TestSetup but runs the server on the main thread
// and clients on separate threads
class ThreadReversedTestSetup : public ::testing::Test
{
protected:
    Server *server = nullptr;
    std::stringstream capturedOutput;
    std::streambuf *originalCoutBuffer;
    bool verboseOutput;
    std::vector<std::thread> clientThreads;
    std::mutex outputMutex;
    std::vector<int> openSockets;
    std::atomic<bool> serverRunning{false};
    std::thread serverThread;
    std::mutex clientsMutex;
    std::condition_variable clientsReady;
    int readyClients = 0;
    int totalClients = 0;

    ThreadReversedTestSetup(bool verbose = true)
        : verboseOutput(verbose)
    {}

    void SetUp() override
    {
        // Redirect cout to our stringstream for capturing server output
        originalCoutBuffer = std::cout.rdbuf();
        std::cout.rdbuf(capturedOutput.rdbuf());

        // Create server in non-blocking mode
        server = new Server(6667, "42", false);

        // Start the server in its own thread
        serverRunning.store(true);
        serverThread = std::thread([this]() {
            try {
                if (this->server) {
                    // Start the server
                    this->server->loop();
                }
            }
            catch (const std::exception &e) {
                std::cerr << "Server exception: " << e.what() << std::endl;
            }
        });

        // Give the server time to initialize - this one is important to keep
        std::this_thread::sleep_for(std::chrono::milliseconds(200));

        if (verboseOutput) {
            std::cerr << "Server started with password 42" << std::endl;
        }
    }

    void TearDown() override
    {
        // Signal server to stop
        serverRunning.store(false);

        // Stop the server if it exists
        if (server) {
            server->shutdown();
        }

        // Join all client threads
        for (auto &t : clientThreads) {
            if (t.joinable()) {
                t.join();
            }
        }

        // Join the server thread
        if (serverThread.joinable()) {
            serverThread.join();
        }

        // Clean up the server object
        delete server;
        server = nullptr;

        // Clean up sockets
        for (auto &socket : openSockets) {
            if (socket >= 0)
                close(socket);
        }

        // Restore original cout
        std::cout.rdbuf(originalCoutBuffer);

        if (verboseOutput) {
            std::cerr << "Server shutdown complete" << std::endl;
            std::cerr << "Captured output: " << std::endl;
            std::cerr << capturedOutput.str() << std::endl;
        }
    }

    std::vector<int> basicSetupMultiple(int numUsers)
    {
        std::vector<int> clients;

        // Set up synchronization for client threads
        {
            std::lock_guard<std::mutex> lock(clientsMutex);
            totalClients = numUsers;
            readyClients = 0;
        }

        // First create all client sockets and add them to the clients vector in correct order
        for (int i = 0; i < numUsers; ++i) {
            int clientSocket = connectClient();
            EXPECT_GT(clientSocket, 0);
            clients.push_back(clientSocket);
        }

        // Now register each client on its own thread with its corresponding nickname
        for (int i = 0; i < numUsers; ++i) {
            int clientSocket = clients[i];
            std::string nickname = "basicUser" + std::to_string(i);

            // Register in a separate thread
            clientThreads.emplace_back([this, clientSocket, nickname, i]() {
                this->registerClient(clientSocket, nickname);

                // Signal that client is ready
                {
                    std::lock_guard<std::mutex> lock(this->clientsMutex);
                    this->readyClients++;
                    if (this->verboseOutput) {
                        std::cerr << "Client " << nickname << " ready, socket " << clientSocket
                                  << " (" << this->readyClients << "/" << this->totalClients << ")"
                                  << std::endl;
                    }
                }
                this->clientsReady.notify_all();

                // Wait for all clients to be ready before joining channel
                {
                    std::unique_lock<std::mutex> lock(this->clientsMutex);
                    this->clientsReady.wait(
                        lock, [this]() { return this->readyClients == this->totalClients; });
                }

                // Join channel #test
                this->sendCommand(clientSocket, "JOIN #test");
            });
        }

        // Wait for all clients to be ready
        {
            std::unique_lock<std::mutex> lock(clientsMutex);
            clientsReady.wait(lock, [this]() { return readyClients == totalClients; });
        }

        // DO NOT join threads here - let them continue running
        // Instead, add a small delay to ensure all JOINs are processed
        std::this_thread::sleep_for(std::chrono::milliseconds(100 * numUsers));

        // Now clear the output to start fresh
        clearServerOutput();

        return clients;
    }

    // Helper function to connect a client
    int connectClient()
    {
        int clientSocket = socket(AF_INET, SOCK_STREAM, 0);
        if (clientSocket < 0) {
            if (verboseOutput)
                std::cerr << "Error creating socket" << std::endl;
            return -1;
        }

        {
            std::lock_guard<std::mutex> lock(outputMutex);
            openSockets.push_back(clientSocket);
        }

        struct sockaddr_in serverAddr;
        memset(&serverAddr, 0, sizeof(serverAddr));
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_port = htons(6667);
        serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");

        // Try connecting multiple times in case the server isn't ready yet
        int retries = 5;
        bool connected = false;

        while (retries > 0 && !connected) {
            if (connect(clientSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0) {
                if (verboseOutput)
                    std::cerr << "Connection attempt failed, retrying... (" << retries << " left)"
                              << std::endl;
                std::this_thread::sleep_for(std::chrono::milliseconds(200));
                retries--;
            }
            else {
                connected = true;
            }
        }

        // Remove unnecessary delay between connections - server should handle multiple connections
        // std::this_thread::sleep_for(std::chrono::milliseconds(100));

        if (!connected) {
            if (verboseOutput)
                std::cerr << "Error connecting to server after multiple attempts" << std::endl;
            close(clientSocket);
            return -1;
        }

        if (verboseOutput)
            std::cerr << "Client connected with socket: " << clientSocket << std::endl;
        return clientSocket;
    }

    // Helper function to register a client with the server
    bool registerClient(int clientSocket, const std::string &nickname,
                        const std::string &username = "testuser",
                        const std::string &realname = "Test User")
    {
        std::string passCommand = "PASS 42\r\n";
        std::string nickCommand = "NICK " + nickname + "\r\n";
        std::string userCommand = "USER " + username + " 0 * :" + realname + "\r\n";

        if (send(clientSocket, passCommand.c_str(), passCommand.length(), 0) < 0) {
            if (verboseOutput)
                std::cerr << "Error sending PASS command" << std::endl;
            return false;
        }

        // No need for delay - server's extractFullMessages handles messages one by one

        if (send(clientSocket, nickCommand.c_str(), nickCommand.length(), 0) < 0) {
            if (verboseOutput)
                std::cerr << "Error sending NICK command" << std::endl;
            return false;
        }

        // No need for delay - server's extractFullMessages handles messages one by one

        if (send(clientSocket, userCommand.c_str(), userCommand.length(), 0) < 0) {
            if (verboseOutput)
                std::cerr << "Error sending USER command" << std::endl;
            return false;
        }

        // We should still wait briefly to ensure registration completes before returning,
        // but we can reduce this further since we use waitForOutput in tests
        std::this_thread::sleep_for(std::chrono::milliseconds(50));

        if (verboseOutput)
            std::cerr << "Client registered with nickname: " << nickname << std::endl;
        return true;
    }

    // Helper function to send a command
    bool sendCommand(int clientSocket, const std::string &command)
    {
        std::string fullCommand = command + "\r\n";

        // Add debug output of which socket is sending which command
        if (verboseOutput) {
            std::cerr << "Socket " << clientSocket << " sending command: " << command << std::endl;
        }

        if (send(clientSocket, fullCommand.c_str(), fullCommand.length(), 0) < 0) {
            if (verboseOutput)
                std::cerr << "Error sending command: " << command << std::endl;
            return false;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(100));

        if (verboseOutput) {
            std::cerr << "Command sent: " << command << std::endl;
        }

        return true;
    }

    std::string getServerOutput()
    {
        std::lock_guard<std::mutex> lock(outputMutex);
        return capturedOutput.str();
    }

    // Helper to clear captured output
    void clearServerOutput()
    {
        std::lock_guard<std::mutex> lock(outputMutex);
        if (verboseOutput)
            std::cerr << capturedOutput.str() << std::endl;
        capturedOutput.str("");
        capturedOutput.clear();
    }

    bool waitForOutput(const std::string &text, int maxWaitMs = 1000)
    {
        auto startTime = std::chrono::steady_clock::now();

        while (true) {
            // Check if text is in current output
            {
                std::string output = getServerOutput();
                if (output.find(text) != std::string::npos) {
                    return true;
                }
            }

            // Check if we've exceeded the timeout
            auto currentTime = std::chrono::steady_clock::now();
            auto elapsedMs =
                std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - startTime)
                    .count();

            if (elapsedMs > maxWaitMs) {
                if (verboseOutput) {
                    std::cerr << "Timeout waiting for output: " << text << std::endl;
                }
                return false;
            }

            // Wait a bit before checking again
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }
    }

    // Override outputContains to use the waiting mechanism
    bool outputContains(const std::string &text)
    {
        return waitForOutput(text);
    }
};
