#include "ThreadSafeMessageBroker.h"
#include "FeedHandler.h"
#include <iostream>
#include <algorithm>

ThreadSafeMessageBroker::ThreadSafeMessageBroker() 
    : running_(false), messageCount_(0), totalLatencyMicros_(0) {
}

ThreadSafeMessageBroker::~ThreadSafeMessageBroker() {
    stop();
}

void ThreadSafeMessageBroker::subscribe(SubscriberType type, MessageCallback callback) {
    std::lock_guard<std::mutex> lock(subscriberMutex_);
    subscribers_[type] = callback;
    std::cout << "Subscriber registered for type: " << static_cast<int>(type) << std::endl;
}

void ThreadSafeMessageBroker::unsubscribe(SubscriberType type) {
    std::lock_guard<std::mutex> lock(subscriberMutex_);
    subscribers_.erase(type);
    std::cout << "Subscriber unregistered for type: " << static_cast<int>(type) << std::endl;
}

void ThreadSafeMessageBroker::publishMessage(const MarketData& data) {
    {
        std::lock_guard<std::mutex> lock(queueMutex_);
        messageQueue_.emplace(data);
    }
    queueCondition_.notify_one();
}

void ThreadSafeMessageBroker::start() {
    if (running_) return;
    
    running_ = true;
    
    // Create worker threads (typically 2-4 threads for optimal performance)
    size_t numThreads = std::thread::hardware_concurrency();
    if (numThreads == 0) numThreads = 2;
    
    for (size_t i = 0; i < numThreads; ++i) {
        workerThreads_.emplace_back(&ThreadSafeMessageBroker::workerThread, this);
    }
    
    std::cout << "Message broker started with " << numThreads << " worker threads" << std::endl;
}

void ThreadSafeMessageBroker::stop() {
    if (!running_) return;
    
    running_ = false;
    queueCondition_.notify_all();
    
    for (auto& thread : workerThreads_) {
        if (thread.joinable()) {
            thread.join();
        }
    }
    
    workerThreads_.clear();
    std::cout << "Message broker stopped" << std::endl;
}

void ThreadSafeMessageBroker::workerThread() {
    while (running_) {
        std::unique_lock<std::mutex> lock(queueMutex_);
        
        // Wait for messages or stop signal
        queueCondition_.wait(lock, [this] { 
            return !messageQueue_.empty() || !running_; 
        });
        
        if (!running_) break;
        
        if (!messageQueue_.empty()) {
            MessageWrapper wrapper = messageQueue_.front();
            messageQueue_.pop();
            lock.unlock();
            
            // Process message with all subscribers
            {
                std::lock_guard<std::mutex> subscriberLock(subscriberMutex_);
                for (const auto& [type, callback] : subscribers_) {
                    try {
                        callback(wrapper.data);
                    } catch (const std::exception& e) {
                        std::cerr << "Error in subscriber callback: " << e.what() << std::endl;
                    }
                }
            }
            
            // Update statistics
            messageCount_++;
            double latency = calculateLatency(wrapper);
            totalLatencyMicros_ += static_cast<uint64_t>(latency * 1000); // Convert to microseconds
            
            // Log high latency messages
            if (latency > 1.0) { // > 1ms
                std::cout << "High latency detected: " << latency << "ms" << std::endl;
            }
        }
    }
}

double ThreadSafeMessageBroker::calculateLatency(const MessageWrapper& wrapper) const {
    auto now = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(
        now - wrapper.timestamp
    );
    return duration.count() / 1000.0; // Convert to milliseconds
}

size_t ThreadSafeMessageBroker::getMessageCount() const {
    return messageCount_;
}

double ThreadSafeMessageBroker::getAverageLatency() const {
    size_t count = messageCount_;
    if (count == 0) return 0.0;
    return static_cast<double>(totalLatencyMicros_) / count / 1000.0; // Convert back to milliseconds
}
