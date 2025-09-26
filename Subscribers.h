#pragma once
#include "FeedHandler.h"
#include <iostream>
#include <vector>
#include <map>
#include <mutex>

// Trading Algorithm Subscriber
class TradingAlgorithmSubscriber {
public:
    TradingAlgorithmSubscriber();
    void onMarketData(const MarketData& data);
    void addSymbol(const std::string& symbol);
    void removeSymbol(const std::string& symbol);
    
    // Trading logic
    void processSignal(const MarketData& data);
    
private:
    std::vector<std::string> subscribedSymbols_;
    std::mutex symbolsMutex_;
    
    // Simple moving average for signal generation
    std::map<std::string, std::vector<double>> priceHistory_;
    std::mutex historyMutex_;
    
    void updatePriceHistory(const std::string& symbol, double price);
    double calculateMovingAverage(const std::string& symbol, int period = 20);
};

// Risk Management Subscriber
class RiskManagementSubscriber {
public:
    RiskManagementSubscriber();
    void onMarketData(const MarketData& data);
    
    // Risk checks
    void checkPriceDeviation(const MarketData& data);
    void checkVolumeSpike(const MarketData& data);
    void checkCircuitBreaker(const MarketData& data);
    
    // Risk limits
    void setPriceDeviationLimit(double limit);
    void setVolumeSpikeThreshold(double threshold);
    
private:
    double priceDeviationLimit_;
    double volumeSpikeThreshold_;
    std::mutex limitsMutex_;
    
    // Track previous prices for deviation calculation
    std::map<std::string, double> lastPrices_;
    std::map<std::string, int> lastVolumes_;
    std::mutex dataMutex_;
};

// Analytics Subscriber
class AnalyticsSubscriber {
public:
    AnalyticsSubscriber();
    void onMarketData(const MarketData& data);
    
    // Analytics functions
    void calculateStatistics(const MarketData& data);
    void generateReports();
    
    // Statistics
    size_t getTotalMessages() const;
    double getAveragePrice(const std::string& symbol) const;
    int getTotalVolume(const std::string& symbol) const;
    
private:
    std::map<std::string, std::vector<double>> priceData_;
    std::map<std::string, std::vector<int>> volumeData_;
    std::atomic<size_t> totalMessages_;
    std::mutex dataMutex_;
};
