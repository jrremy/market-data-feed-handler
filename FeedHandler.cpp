#include "FeedHandler.h"
#include "ThreadSafeMessageBroker.h"
#include <iostream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <sstream>
#include <chrono>

FeedHandler::FeedHandler(const std::string& host, int port)
    : host_(host), port_(port), sockfd_(-1), running_(false), 
      messagesProcessed_(0), totalProcessingTimeMicros_(0) {}

FeedHandler::~FeedHandler() {
    stop();
}

void FeedHandler::setMessageBroker(std::shared_ptr<ThreadSafeMessageBroker> broker) {
    messageBroker_ = broker;
}

void FeedHandler::start() {
    if (running_) return;
    
    sockfd_ = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd_ < 0) {
        std::cerr << "Socket creation failed\n";
        return;
    }
    
    sockaddr_in serv_addr{};
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port_);
    inet_pton(AF_INET, host_.c_str(), &serv_addr.sin_addr);

    if (connect(sockfd_, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        std::cerr << "Connection failed\n";
        close(sockfd_);
        sockfd_ = -1;
        return;
    }
    
    running_ = true;
    networkThread_ = std::thread(&FeedHandler::networkThreadFunction, this);
    std::cout << "FeedHandler started, connected to " << host_ << ":" << port_ << std::endl;
}

void FeedHandler::stop() {
    if (!running_) return;
    
    running_ = false;
    
    if (sockfd_ >= 0) {
        close(sockfd_);
        sockfd_ = -1;
    }
    
    if (networkThread_.joinable()) {
        networkThread_.join();
    }
    
    std::cout << "FeedHandler stopped" << std::endl;
}

void FeedHandler::networkThreadFunction() {
    char buffer[4096]; // Larger buffer for high throughput
    std::string messageBuffer;
    
    while (running_) {
        ssize_t n = read(sockfd_, buffer, sizeof(buffer) - 1);
        if (n <= 0) {
            if (running_) {
                std::cerr << "Connection lost or error reading from socket\n";
            }
            break;
        }
        
        buffer[n] = '\0';
        messageBuffer += std::string(buffer);
        
        // Process complete messages (assuming newline-delimited)
        size_t pos = 0;
        while ((pos = messageBuffer.find('\n')) != std::string::npos) {
            std::string message = messageBuffer.substr(0, pos);
            messageBuffer.erase(0, pos + 1);
            
            if (!message.empty()) {
                processMessage(message);
            }
        }
    }
}

void FeedHandler::processMessage(const std::string& msg) {
    auto start = std::chrono::high_resolution_clock::now();
    
    MarketData data;
    if (parseMarketData(msg, data)) {
        // Publish to message broker if available
        if (messageBroker_) {
            messageBroker_->publishMessage(data);
        }
        
        messagesProcessed_++;
        
        // Calculate processing time
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        totalProcessingTimeMicros_ += duration.count();
    }
}

bool FeedHandler::parseMarketData(const std::string& msg, MarketData& data) {
    try {
        std::istringstream ss(msg);
        std::string symbol, price_str, size_str, timestamp_str;
        
        // Parse CSV format: symbol,price,size,timestamp
        if (std::getline(ss, symbol, ',') &&
            std::getline(ss, price_str, ',') &&
            std::getline(ss, size_str, ',') &&
            std::getline(ss, timestamp_str)) {
            
            data.symbol = symbol;
            data.price = std::stod(price_str);
            data.size = std::stoi(size_str);
            data.timestamp = timestamp_str;
            
            return true;
        } else {
            std::cerr << "Malformed message: " << msg << std::endl;
            return false;
        }
    } catch (const std::exception& e) {
        std::cerr << "Parse error: " << e.what() << " in message: " << msg << std::endl;
        return false;
    }
}

size_t FeedHandler::getMessagesProcessed() const {
    return messagesProcessed_;
}

double FeedHandler::getAverageProcessingTime() const {
    size_t count = messagesProcessed_;
    if (count == 0) return 0.0;
    return static_cast<double>(totalProcessingTimeMicros_) / count / 1000.0; // Convert to milliseconds
}