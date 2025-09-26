#pragma once
#include <string>
#include <vector>
#include <functional>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <thread>
#include <atomic>
#include <memory>
#include <map>
#include <chrono>

// Include MarketData definition
#include "FeedHandler.h"

// Callback function type for message processing
using MessageCallback = std::function<void(const MarketData&)>;

// Subscriber types for different components
enum class SubscriberType {
    TRADING_ALGORITHM,
    RISK_MANAGEMENT,
    ANALYTICS
};

class ThreadSafeMessageBroker {
public:
    ThreadSafeMessageBroker();
    ~ThreadSafeMessageBroker();
    
    // Subscription management
    void subscribe(SubscriberType type, MessageCallback callback);
    void unsubscribe(SubscriberType type);
    
    // Message publishing
    void publishMessage(const MarketData& data);
    
    // Control
    void start();
    void stop();
    
    // Statistics
    size_t getMessageCount() const;
    double getAverageLatency() const;

private:
    struct MessageWrapper {
        MarketData data;
        std::chrono::high_resolution_clock::time_point timestamp;
        
        MessageWrapper(const MarketData& d) 
            : data(d), timestamp(std::chrono::high_resolution_clock::now()) {}
    };
    
    // Thread-safe message queue
    std::queue<MessageWrapper> messageQueue_;
    std::mutex queueMutex_;
    std::condition_variable queueCondition_;
    
    // Subscriber management
    std::map<SubscriberType, MessageCallback> subscribers_;
    std::mutex subscriberMutex_;
    
    // Worker threads
    std::vector<std::thread> workerThreads_;
    std::atomic<bool> running_;
    
    // Statistics
    std::atomic<size_t> messageCount_;
    std::atomic<uint64_t> totalLatencyMicros_;
    std::mutex statsMutex_;
    
    // Worker thread function
    void workerThread();
    
    // Calculate latency
    double calculateLatency(const MessageWrapper& wrapper) const;
};
