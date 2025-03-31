#include "GeminiBot.hpp"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#include <sstream>
#include <cstring>
#include <fcntl.h>
#include <poll.h>
#include <thread>
#include <iostream>
#include <fstream>
#include <cstdlib>

GeminiBot::GeminiBot(const std::string &server, int port, const std::string &password,
                     const std::string &nickname, const std::string &api_key)
    : IRCBot(server, port, password, nickname, "#hive42")
    , _api_key(api_key)
{}

GeminiBot::~GeminiBot()
{}

void GeminiBot::handleMessage(const std::string &message)
{
    if (message.find("JOIN") != std::string::npos) {
        processJoinMessage(message);
    }
}

void GeminiBot::processJoinMessage(const std::string &joinMessage)
{
    std::string joiningNick = parseNicknameFromJoin(joinMessage);
    if (!joiningNick.empty() && joiningNick != _nickname) {
        std::string greeting = generateGreeting(joiningNick);
        if (!greeting.empty()) {
            queueMessage("#hive42", greeting);
        }
    }
}

void GeminiBot::queueMessage(const std::string &channel, const std::string &message)
{
    std::lock_guard<std::mutex> lock(_queueMutex);
    _messageQueue.push("PRIVMSG " + channel + " :" + message);

    // Process queue
    while (!_messageQueue.empty()) {
        std::string msg = _messageQueue.front();
        sendRaw(msg);
        _messageQueue.pop();
        // Add delay between messages to prevent flooding
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }
}

bool GeminiBot::connectToApi(int &sock)
{
    std::cout << "[DEBUG] Connecting to Gemini API..." << std::endl;

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        std::cerr << "[ERROR] Failed to create API socket: " << strerror(errno) << std::endl;
        return false;
    }
    std::cout << "[DEBUG] API socket created successfully: " << sock << std::endl;

    // Set socket to non-blocking mode
    int flags = fcntl(sock, F_GETFL, 0);
    fcntl(sock, F_SETFL, flags | O_NONBLOCK);
    std::cout << "[DEBUG] Set API socket to non-blocking mode" << std::endl;

    struct hostent *host = gethostbyname("generativelanguage.googleapis.com");
    if (!host) {
        std::cerr << "[ERROR] Failed to resolve API hostname: " << strerror(errno) << std::endl;
        close(sock);
        return false;
    }
    std::cout << "[DEBUG] Resolved API hostname to " << inet_ntoa(*(struct in_addr *)host->h_addr)
              << std::endl;

    struct sockaddr_in serverAddr;
    std::memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(443); // HTTPS port
    std::memcpy(&serverAddr.sin_addr, host->h_addr, host->h_length);

    // Non-blocking connect
    std::cout << "[DEBUG] Attempting to connect to API..." << std::endl;
    if (::connect(sock, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0) {
        if (errno != EINPROGRESS) {
            std::cerr << "[ERROR] API connection failed immediately: " << strerror(errno)
                      << std::endl;
            close(sock);
            return false;
        }
        std::cout << "[DEBUG] API connection in progress..." << std::endl;
    }
    else {
        std::cout << "[DEBUG] API connected immediately!" << std::endl;
    }

    // Wait for connection with timeout
    struct pollfd pfd;
    pfd.fd = sock;
    pfd.events = POLLOUT;

    std::cout << "[DEBUG] Waiting for API connection to complete (timeout: " << API_TIMEOUT
              << "s)..." << std::endl;
    int poll_result = poll(&pfd, 1, API_TIMEOUT * 1000);

    if (poll_result <= 0) {
        std::cerr << "[ERROR] API connection timeout or error: "
                  << (poll_result == 0 ? "timeout" : strerror(errno)) << std::endl;
        close(sock);
        return false;
    }

    // Check if connection was successful
    int error = 0;
    socklen_t len = sizeof(error);
    if (getsockopt(sock, SOL_SOCKET, SO_ERROR, &error, &len) < 0 || error != 0) {
        std::cerr << "[ERROR] API connection failed: "
                  << (getsockopt(sock, SOL_SOCKET, SO_ERROR, &error, &len) < 0 ? strerror(errno)
                                                                               : strerror(error))
                  << std::endl;
        close(sock);
        return false;
    }

    // Set back to blocking mode
    std::cout << "[DEBUG] API connection established, setting back to blocking mode" << std::endl;
    fcntl(sock, F_SETFL, flags);
    return true;
}

std::string GeminiBot::sendApiRequest(int sock, const std::string &prompt)
{
    std::cout << "[DEBUG] Preparing API request for prompt: " << prompt << std::endl;

    std::string jsonBody = "{\"contents\":[{\"parts\":[{\"text\":\"" + prompt + "\"}]}]}";
    std::string request = "POST /v1beta/models/gemini-2.0-flash:generateContent?key=" + _api_key +
                          " HTTP/1.1\r\n"
                          "Host: generativelanguage.googleapis.com\r\n"
                          "Content-Type: application/json\r\n"
                          "Content-Length: " +
                          std::to_string(jsonBody.length()) +
                          "\r\n"
                          "\r\n" +
                          jsonBody;

    // Set timeout for send/receive
    struct timeval tv;
    tv.tv_sec = API_TIMEOUT;
    tv.tv_usec = 0;
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
    setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));
    std::cout << "[DEBUG] Set API socket timeouts to " << API_TIMEOUT << " seconds" << std::endl;

    std::cout << "[DEBUG] Sending API request (" << request.length() << " bytes)" << std::endl;
    int sent = send(sock, request.c_str(), request.length(), 0);
    if (sent < 0) {
        std::cerr << "[ERROR] Failed to send API request: " << strerror(errno) << std::endl;
        return "";
    }
    std::cout << "[DEBUG] Successfully sent " << sent << " bytes to API" << std::endl;

    // Add a delay to give the server time to process and respond
    std::cout << "[DEBUG] Waiting 2 seconds for response..." << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(2));

    char buffer[4096];
    std::string response;
    int bytesRead;
    int totalBytesRead = 0;

    std::cout << "[DEBUG] Receiving API response..." << std::endl;
    while ((bytesRead = recv(sock, buffer, sizeof(buffer) - 1, 0)) > 0) {
        buffer[bytesRead] = '\0';
        response += buffer;
        totalBytesRead += bytesRead;
        std::cout << "[DEBUG] Received " << bytesRead << " bytes..." << std::endl;
    }

    if (bytesRead < 0 && errno != EAGAIN && errno != EWOULDBLOCK) {
        std::cerr << "[ERROR] Error receiving API response: " << strerror(errno) << std::endl;
    }

    if (totalBytesRead == 0) {
        std::cerr << "[ERROR] Received empty response from API" << std::endl;
        return "";
    }

    std::cout << "[DEBUG] Received total of " << totalBytesRead << " bytes from API" << std::endl;
    return response;
}

std::string GeminiBot::parseNicknameFromJoin(const std::string &joinMessage)
{
    // JOIN message format: ":nickname!username@host JOIN #channel"
    size_t nickStart = 1; // Skip ':'
    size_t nickEnd = joinMessage.find('!');
    if (nickEnd != std::string::npos) {
        return joinMessage.substr(nickStart, nickEnd - nickStart);
    }
    return "";
}

std::string GeminiBot::parseApiResponse(const std::string &response)
{
    std::cout << "[DEBUG] Parsing API response of length " << response.length() << std::endl;

    size_t bodyStart = response.find("\r\n\r\n");
    if (bodyStart != std::string::npos) {
        std::string body = response.substr(bodyStart + 4);
        std::cout << "[DEBUG] API response body: " << body << std::endl;

        size_t textStart = body.find("\"text\": \"");
        if (textStart != std::string::npos) {
            textStart += 9;
            size_t textEnd = body.find("\"", textStart);
            if (textEnd != std::string::npos) {
                std::string parsedText = body.substr(textStart, textEnd - textStart);
                std::cout << "[DEBUG] Extracted text from API response: " << parsedText
                          << std::endl;
                return parsedText;
            }
            else {
                std::cerr << "[ERROR] Could not find end quote for text field" << std::endl;
            }
        }
        else {
            std::cerr << "[ERROR] Could not find text field in API response" << std::endl;
        }
    }
    else {
        std::cerr << "[ERROR] Could not find body in API response" << std::endl;
    }

    std::cout << "[DEBUG] Failed to parse API response" << std::endl;
    return "";
}

std::string GeminiBot::makeApiRequest(const std::string &prompt)
{
    std::cout << "[DEBUG] Making API request through curl command..." << std::endl;

    // Escape quotes in prompt
    std::string escapedPrompt = prompt;
    size_t pos = 0;
    while ((pos = escapedPrompt.find("\"", pos)) != std::string::npos) {
        escapedPrompt.replace(pos, 1, "\\\"");
        pos += 2;
    }

    // Create temporary files for output and command
    std::string tmpFile = "/tmp/gemini_response_" + std::to_string(time(NULL)) + ".json";
    std::string cmd = "curl -s "
                      "\"https://generativelanguage.googleapis.com/v1beta/models/"
                      "gemini-2.0-flash:generateContent?key=" +
                      _api_key + "\" " + "-H 'Content-Type: application/json' " + "-X POST " +
                      "-d '{\"contents\":[{\"parts\":[{\"text\": \"" + escapedPrompt + "\"}]}]}' " +
                      "> " + tmpFile;

    std::cout << "[DEBUG] Executing: " << cmd << std::endl;
    int result = system(cmd.c_str());

    if (result != 0) {
        std::cerr << "[ERROR] Command failed with code " << result << std::endl;
        return "";
    }

    // Read the response file
    std::ifstream file(tmpFile);
    if (!file.is_open()) {
        std::cerr << "[ERROR] Failed to open response file" << std::endl;
        return "";
    }

    std::string response((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    file.close();

    // Clean up temp file
    std::remove(tmpFile.c_str());

    if (response.empty()) {
        std::cerr << "[ERROR] Empty response in file" << std::endl;
        return "";
    }

    std::cout << "[DEBUG] Got response: " << response << std::endl;

    // Parse the JSON response
    size_t textPos = response.find("\"text\":");
    if (textPos == std::string::npos) {
        std::cerr << "[ERROR] Could not find 'text' field in response" << std::endl;
        return "";
    }

    textPos += 8; // Move past "\"text\": "

    // Find the opening quote
    textPos = response.find("\"", textPos);
    if (textPos == std::string::npos) {
        std::cerr << "[ERROR] Could not find opening quote for text value" << std::endl;
        return "";
    }
    textPos++; // Move past the opening quote

    // Find the closing quote
    size_t endPos = textPos;

    while (true) {
        endPos = response.find("\"", endPos);
        if (endPos == std::string::npos)
            break;

        // Check if this quote is escaped
        if (response[endPos - 1] == '\\') {
            endPos++;
            continue;
        }

        // If we got here, we found an unescaped quote
        break;
    }

    if (endPos == std::string::npos) {
        std::cerr << "[ERROR] Could not find closing quote for text value" << std::endl;
        return "";
    }

    std::string text = response.substr(textPos, endPos - textPos);

    // Unescape any escaped quotes
    pos = 0;
    while ((pos = text.find("\\\"", pos)) != std::string::npos) {
        text.replace(pos, 2, "\"");
        pos += 1;
    }

    std::cout << "[DEBUG] Extracted text: " << text << std::endl;
    return text;
}

std::string GeminiBot::generateGreeting(const std::string &username)
{
    std::string prompt = "Generate a short, friendly IRC greeting message for user " + username +
                         ". Keep it under 50 characters and make it casual and fun.";

    std::string response = makeApiRequest(prompt);

    if (response.empty()) {
        return "Welcome, " + username + "!";
    }

    return response;
}
