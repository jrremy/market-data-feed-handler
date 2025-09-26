#pragma once
#include <string>
#include <memory>
#include <thread>
#include <atomic>

// Forward declaration
class ThreadSafeMessageBroker;

struct MarketData {
    std::string symbol;
    double price;
    int size;
    std::string timestamp;
};

class FeedHandler {
public:
    FeedHandler(const std::string& host, int port);
    ~FeedHandler();
    
    void start();
    void stop();
    void processMessage(const std::string& msg);
    
    // Set the message broker for publishing
    void setMessageBroker(std::shared_ptr<ThreadSafeMessageBroker> broker);
    
    // Statistics
    size_t getMessagesProcessed() const;
    double getAverageProcessingTime() const;

private:
    std::string host_;
    int port_;
    int sockfd_;
    std::atomic<bool> running_;
    std::thread networkThread_;
    
    // Message broker for publishing
    std::shared_ptr<ThreadSafeMessageBroker> messageBroker_;
    
    // Statistics
    std::atomic<size_t> messagesProcessed_;
    std::atomic<uint64_t> totalProcessingTimeMicros_;
    
    // Network thread function
    void networkThreadFunction();
    
    // Parse message with error handling
    bool parseMarketData(const std::string& msg, MarketData& data);
};