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

class TestSetup : public ::testing::Test
{
protected:
    Server server;
    std::stringstream capturedOutput;
    std::streambuf *originalCoutBuffer;
    bool verboseOutput;
    std::thread serverThread;
    std::mutex outputMutex;

    TestSetup(bool verbose = false)
        : verboseOutput(verbose)
    {}

    void SetUp() override
    {
        // Redirect cout to our stringstream for capturing server output
        originalCoutBuffer = std::cout.rdbuf();
        std::cout.rdbuf(capturedOutput.rdbuf());

        // Start the server in a separate thread
        serverThread = std::thread([this]() { server.start("42"); });

        // Add a delay to ensure the server is fully running
        std::this_thread::sleep_for(std::chrono::milliseconds(200));

        if (verboseOutput) {
            std::cerr << "Server started with password 42" << std::endl;
        }
    }

    void TearDown() override
    {
        // Stop the server
        server.shutdown();

        // Join the server thread
        if (serverThread.joinable()) {
            serverThread.join();
        }

        // Restore original cout
        std::cout.rdbuf(originalCoutBuffer);

        if (verboseOutput) {
            std::cerr << "Server shutdown complete" << std::endl;
            std::cerr << "Captured output: " << std::endl;
            std::cerr << capturedOutput.str() << std::endl;
        }
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

        // a little wait between connections
        std::this_thread::sleep_for(std::chrono::milliseconds(50));

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

        std::this_thread::sleep_for(std::chrono::milliseconds(50));

        if (send(clientSocket, nickCommand.c_str(), nickCommand.length(), 0) < 0) {
            if (verboseOutput)
                std::cerr << "Error sending NICK command" << std::endl;
            return false;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(50));

        if (send(clientSocket, userCommand.c_str(), userCommand.length(), 0) < 0) {
            if (verboseOutput)
                std::cerr << "Error sending USER command" << std::endl;
            return false;
        }

        // Wait for registration to complete
        std::this_thread::sleep_for(std::chrono::milliseconds(200));

        if (verboseOutput)
            std::cerr << "Client registered with nickname: " << nickname << std::endl;
        return true;
    }

    // Helper function to send a command
    bool sendCommand(int clientSocket, const std::string &command)
    {
        std::string fullCommand = command + "\r\n";
        if (send(clientSocket, fullCommand.c_str(), fullCommand.length(), 0) < 0) {
            if (verboseOutput)
                std::cerr << "Error sending command: " << command << std::endl;
            return false;
        }

        // Add a small delay to give the server time to process
        std::this_thread::sleep_for(std::chrono::milliseconds(50));

        if (verboseOutput) {
            std::cerr << "Command sent: " << command << std::endl;
        }

        return true;
    }

    // Thread-safe function to run client operations
    template <typename Func> bool runClientOperation(const std::string &nickname, Func operation)
    {
        int clientSocket = connectClient();
        if (clientSocket < 0) {
            return false;
        }

        bool result = registerClient(clientSocket, nickname);
        if (!result) {
            close(clientSocket);
            return false;
        }

        result = operation(clientSocket);

        close(clientSocket);
        return result;
    }

    // Helper for running multi-client tests with concurrent clients
    void
    runMultiClientTest(const std::vector<std::string> &nicknames,
                       const std::function<void(int socketFd, const std::string &nick)> &clientFunc)
    {
        std::vector<std::thread> clientThreads;
        std::atomic<int> successCount{0};

        // Start all client threads
        for (const auto &nick : nicknames) {
            clientThreads.emplace_back([this, &nick, &clientFunc, &successCount]() {
                int socketFd = connectClient();
                if (socketFd < 0) {
                    return;
                }

                if (registerClient(socketFd, nick)) {
                    clientFunc(socketFd, nick);
                    successCount++;
                }

                close(socketFd);
            });

            // Slight stagger to avoid connection issues
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }

        // Wait for all client threads to complete
        for (auto &thread : clientThreads) {
            if (thread.joinable()) {
                thread.join();
            }
        }

        // Verify all clients succeeded
        EXPECT_EQ(successCount.load(), nicknames.size());
    }

    // Helper function to get captured server output
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

    bool outputContains(const std::string &text)
    {
        std::string output = getServerOutput();
        bool contains = output.find(text) != std::string::npos;

        if (verboseOutput && !contains) {
            std::cerr << "Text not found in output: " << text << std::endl;
        }

        return contains;
    }
};
