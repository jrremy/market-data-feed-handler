#include <iostream>
#include <memory>
#include <thread>
#include <chrono>
#include <signal.h>
#include "FeedHandler.h"
#include "ThreadSafeMessageBroker.h"
#include "Subscribers.h"

// Global variables for cleanup
std::shared_ptr<FeedHandler> g_feedHandler;
std::shared_ptr<ThreadSafeMessageBroker> g_messageBroker;
std::shared_ptr<TradingAlgorithmSubscriber> g_tradingSub;
std::shared_ptr<RiskManagementSubscriber> g_riskSub;
std::shared_ptr<AnalyticsSubscriber> g_analyticsSub;

// Signal handler for graceful shutdown
void signalHandler(int signal) {
    std::cout << "\nReceived signal " << signal << ", shutting down gracefully..." << std::endl;
    
    if (g_feedHandler) {
        g_feedHandler->stop();
    }
    
    if (g_messageBroker) {
        g_messageBroker->stop();
    }
    
    if (g_analyticsSub) {
        g_analyticsSub->generateReports();
    }
    
    std::cout << "Shutdown complete." << std::endl;
    exit(0);
}

int main() {
    // Set up signal handlers
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);
    
    std::cout << "=== Market Data Feed Handler ===" << std::endl;
    std::cout << "Features:" << std::endl;
    std::cout << "- Thread-safe message distribution" << std::endl;
    std::cout << "- Multiple subscribers (Trading, Risk, Analytics)" << std::endl;
    std::cout << "- Sub-millisecond latency processing" << std::endl;
    std::cout << "- Async processing queues" << std::endl;
    std::cout << "- Callback-based subscriptions" << std::endl;
    std::cout << "- High throughput (10,000+ msg/sec)" << std::endl;
    std::cout << "- Zero message loss with error handling" << std::endl;
    std::cout << "================================\n" << std::endl;
    
    try {
        // Create message broker
        g_messageBroker = std::make_shared<ThreadSafeMessageBroker>();
        
        // Create subscribers
        g_tradingSub = std::make_shared<TradingAlgorithmSubscriber>();
        g_riskSub = std::make_shared<RiskManagementSubscriber>();
        g_analyticsSub = std::make_shared<AnalyticsSubscriber>();
        
        // Subscribe to message broker
        g_messageBroker->subscribe(SubscriberType::TRADING_ALGORITHM, 
            [&](const MarketData& data) { g_tradingSub->onMarketData(data); });
        
        g_messageBroker->subscribe(SubscriberType::RISK_MANAGEMENT, 
            [&](const MarketData& data) { g_riskSub->onMarketData(data); });
        
        g_messageBroker->subscribe(SubscriberType::ANALYTICS, 
            [&](const MarketData& data) { g_analyticsSub->onMarketData(data); });
        
        // Configure trading algorithm
        g_tradingSub->addSymbol("AAPL");
        g_tradingSub->addSymbol("GOOGL");
        g_tradingSub->addSymbol("MSFT");
        
        // Configure risk management
        g_riskSub->setPriceDeviationLimit(5.0);  // 5% price deviation limit
        g_riskSub->setVolumeSpikeThreshold(3.0); // 3x volume spike threshold
        
        // Start message broker
        g_messageBroker->start();
        
        // Create and configure feed handler
        g_feedHandler = std::make_shared<FeedHandler>("127.0.0.1", 9000);
        g_feedHandler->setMessageBroker(g_messageBroker);
        
        // Start feed handler
        g_feedHandler->start();
        
        std::cout << "System started successfully!" << std::endl;
        std::cout << "Press Ctrl+C to stop and generate reports." << std::endl;
        
        // Main loop - monitor performance
        auto startTime = std::chrono::high_resolution_clock::now();
        size_t lastMessageCount = 0;
        
        while (true) {
            std::this_thread::sleep_for(std::chrono::seconds(5));
            
            // Performance monitoring
            size_t currentMessages = g_feedHandler->getMessagesProcessed();
            size_t brokerMessages = g_messageBroker->getMessageCount();
            double avgLatency = g_messageBroker->getAverageLatency();
            double avgProcessingTime = g_feedHandler->getAverageProcessingTime();
            
            auto now = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::seconds>(now - startTime);
            
            size_t messagesPerSecond = (currentMessages - lastMessageCount) / 5;
            
            std::cout << "\n=== PERFORMANCE STATS ===" << std::endl;
            std::cout << "Runtime: " << duration.count() << " seconds" << std::endl;
            std::cout << "Messages Processed: " << currentMessages << std::endl;
            std::cout << "Messages Published: " << brokerMessages << std::endl;
            std::cout << "Current Rate: " << messagesPerSecond << " msg/sec" << std::endl;
            std::cout << "Average Latency: " << avgLatency << " ms" << std::endl;
            std::cout << "Average Processing Time: " << avgProcessingTime << " ms" << std::endl;
            std::cout << "========================\n" << std::endl;
            
            lastMessageCount = currentMessages;
            
            // Check for high throughput
            if (messagesPerSecond > 10000) {
                std::cout << "HIGH THROUGHPUT ACHIEVED: " << messagesPerSecond << " msg/sec!" << std::endl;
            }
            
            // Check for sub-millisecond latency
            if (avgLatency < 1.0 && avgLatency > 0) {
                std::cout << "SUB-MILLISECOND LATENCY ACHIEVED: " << avgLatency << " ms!" << std::endl;
            }
        }
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}