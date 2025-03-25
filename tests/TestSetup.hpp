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

#define SERVER_STARTUP_DELAY 5
#define SEND_COMMAND_DELAY 1
#define RECHECK_OUTPUT_DELAY 5
#define MAX_WAIT_OUTPUT 200

// Runs the test on the main thread
// Runs server and clients on separate threads
class TestSetup : public ::testing::Test
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

    TestSetup(bool verbose = true)
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

        // Give the server time to initialize
        std::this_thread::sleep_for(std::chrono::milliseconds(SERVER_STARTUP_DELAY));

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

        // First create all client sockets and add them to the clients vector in correct order
        for (int i = 0; i < numUsers; ++i) {
            int clientSocket = connectClient();
            EXPECT_GT(clientSocket, 0);
            clients.push_back(clientSocket);
        }

        // Set up thread synchronization
        {
            std::lock_guard<std::mutex> lock(clientsMutex);
            totalClients = numUsers;
            readyClients = 0;
        }

        // Register clients
        for (int i = 0; i < numUsers; ++i) {
            int clientSocket = clients[i];
            std::string nickname = "basicUser" + std::to_string(i);

            clientThreads.emplace_back([this, clientSocket, nickname]() {
                // Register client
                this->registerClient(clientSocket, nickname);

                // Signal that registration is complete
                {
                    std::lock_guard<std::mutex> lock(this->clientsMutex);
                    this->readyClients++;
                }
                this->clientsReady.notify_all();

                // Wait for all clients to be ready
                {
                    std::unique_lock<std::mutex> lock(this->clientsMutex);
                    this->clientsReady.wait(
                        lock, [this]() { return this->readyClients == this->totalClients; });
                }
            });
        }

        // Wait for all clients to be registered
        {
            std::unique_lock<std::mutex> lock(clientsMutex);
            clientsReady.wait(lock, [this]() { return readyClients == totalClients; });
        }

        for (int i = 0; i < numUsers; ++i) {
            sendCommand(clients[i], "JOIN #test");
        }

        // Clear output before returning
        clearServerOutput();

        return clients;
    }

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

        if (connect(clientSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0) {
            if (verboseOutput)
                std::cerr << "Error connecting to server" << std::endl;
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

        if (send(clientSocket, passCommand.c_str(), passCommand.length(), 0) < 0 ||
            send(clientSocket, nickCommand.c_str(), nickCommand.length(), 0) < 0 ||
            send(clientSocket, userCommand.c_str(), userCommand.length(), 0) < 0) {
            if (verboseOutput)
                std::cerr << "Error sending registration commands" << std::endl;
            return false;
        }

        if (verboseOutput)
            std::cerr << "Client registered with nickname: " << nickname << std::endl;
        return true;
    }

    // Helper function to send a command
    bool sendCommand(int clientSocket, const std::string &command)
    {
        // allow commands to process in correct order from test
        std::this_thread::sleep_for(std::chrono::milliseconds(SEND_COMMAND_DELAY));
        std::string fullCommand = command + "\r\n";

        if (verboseOutput) {
            std::cerr << "Socket " << clientSocket << " sending command: " << command << std::endl;
        }

        if (send(clientSocket, fullCommand.c_str(), fullCommand.length(), 0) < 0) {
            if (verboseOutput)
                std::cerr << "Error sending command: " << command << std::endl;
            return false;
        }

        if (verboseOutput)
            std::cerr << "Command sent: " << command << std::endl;
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

    bool waitForOutput(const std::string &text, int maxWaitMs = MAX_WAIT_OUTPUT)
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
            std::this_thread::sleep_for(std::chrono::milliseconds(RECHECK_OUTPUT_DELAY));
        }
    }

    // Override outputContains to use the waiting mechanism
    bool outputContains(const std::string &text)
    {
        return waitForOutput(text);
    }
};
