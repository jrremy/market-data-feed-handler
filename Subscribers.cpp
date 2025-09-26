#include "Subscribers.h"
#include <algorithm>
#include <numeric>
#include <cmath>

// Trading Algorithm Subscriber Implementation
TradingAlgorithmSubscriber::TradingAlgorithmSubscriber() {
    std::cout << "Trading Algorithm Subscriber initialized" << std::endl;
}

void TradingAlgorithmSubscriber::onMarketData(const MarketData& data) {
    std::lock_guard<std::mutex> lock(symbolsMutex_);
    
    // Check if we're subscribed to this symbol
    if (std::find(subscribedSymbols_.begin(), subscribedSymbols_.end(), data.symbol) 
        != subscribedSymbols_.end()) {
        processSignal(data);
    }
}

void TradingAlgorithmSubscriber::addSymbol(const std::string& symbol) {
    std::lock_guard<std::mutex> lock(symbolsMutex_);
    if (std::find(subscribedSymbols_.begin(), subscribedSymbols_.end(), symbol) 
        == subscribedSymbols_.end()) {
        subscribedSymbols_.push_back(symbol);
        std::cout << "Trading Algorithm subscribed to: " << symbol << std::endl;
    }
}

void TradingAlgorithmSubscriber::removeSymbol(const std::string& symbol) {
    std::lock_guard<std::mutex> lock(symbolsMutex_);
    subscribedSymbols_.erase(
        std::remove(subscribedSymbols_.begin(), subscribedSymbols_.end(), symbol),
        subscribedSymbols_.end()
    );
    std::cout << "Trading Algorithm unsubscribed from: " << symbol << std::endl;
}

void TradingAlgorithmSubscriber::processSignal(const MarketData& data) {
    updatePriceHistory(data.symbol, data.price);
    
    double movingAvg = calculateMovingAverage(data.symbol);
    if (movingAvg > 0) {
        double deviation = (data.price - movingAvg) / movingAvg * 100;
        
        if (deviation > 2.0) {
            std::cout << "BUY SIGNAL: " << data.symbol 
                      << " Price: " << data.price 
                      << " MA: " << movingAvg 
                      << " Deviation: " << deviation << "%" << std::endl;
        } else if (deviation < -2.0) {
            std::cout << "SELL SIGNAL: " << data.symbol 
                      << " Price: " << data.price 
                      << " MA: " << movingAvg 
                      << " Deviation: " << deviation << "%" << std::endl;
        }
    }
}

void TradingAlgorithmSubscriber::updatePriceHistory(const std::string& symbol, double price) {
    std::lock_guard<std::mutex> lock(historyMutex_);
    priceHistory_[symbol].push_back(price);
    
    // Keep only last 50 prices for memory efficiency
    if (priceHistory_[symbol].size() > 50) {
        priceHistory_[symbol].erase(priceHistory_[symbol].begin());
    }
}

double TradingAlgorithmSubscriber::calculateMovingAverage(const std::string& symbol, int period) {
    std::lock_guard<std::mutex> lock(historyMutex_);
    
    if (priceHistory_[symbol].size() < period) {
        return 0.0;
    }
    
    auto& prices = priceHistory_[symbol];
    auto start = prices.end() - period;
    return std::accumulate(start, prices.end(), 0.0) / period;
}

// Risk Management Subscriber Implementation
RiskManagementSubscriber::RiskManagementSubscriber() 
    : priceDeviationLimit_(10.0), volumeSpikeThreshold_(5.0) {
    std::cout << "Risk Management Subscriber initialized" << std::endl;
}

void RiskManagementSubscriber::onMarketData(const MarketData& data) {
    checkPriceDeviation(data);
    checkVolumeSpike(data);
    checkCircuitBreaker(data);
}

void RiskManagementSubscriber::checkPriceDeviation(const MarketData& data) {
    std::lock_guard<std::mutex> lock(dataMutex_);
    
    if (lastPrices_.find(data.symbol) != lastPrices_.end()) {
        double lastPrice = lastPrices_[data.symbol];
        double deviation = std::abs(data.price - lastPrice) / lastPrice * 100;
        
        if (deviation > priceDeviationLimit_) {
            std::cout << "RISK ALERT: Price deviation " << deviation 
                      << "% for " << data.symbol << std::endl;
        }
    }
    
    lastPrices_[data.symbol] = data.price;
}

void RiskManagementSubscriber::checkVolumeSpike(const MarketData& data) {
    std::lock_guard<std::mutex> lock(dataMutex_);
    
    if (lastVolumes_.find(data.symbol) != lastVolumes_.end()) {
        int lastVolume = lastVolumes_[data.symbol];
        if (lastVolume > 0) {
            double volumeRatio = static_cast<double>(data.size) / lastVolume;
            
            if (volumeRatio > volumeSpikeThreshold_) {
                std::cout << "RISK ALERT: Volume spike " << volumeRatio 
                          << "x for " << data.symbol << std::endl;
            }
        }
    }
    
    lastVolumes_[data.symbol] = data.size;
}

void RiskManagementSubscriber::checkCircuitBreaker(const MarketData& data) {
    // Simple circuit breaker logic
    if (data.price <= 0) {
        std::cout << "CIRCUIT BREAKER: Invalid price for " << data.symbol << std::endl;
    }
}

void RiskManagementSubscriber::setPriceDeviationLimit(double limit) {
    std::lock_guard<std::mutex> lock(limitsMutex_);
    priceDeviationLimit_ = limit;
}

void RiskManagementSubscriber::setVolumeSpikeThreshold(double threshold) {
    std::lock_guard<std::mutex> lock(limitsMutex_);
    volumeSpikeThreshold_ = threshold;
}

// Analytics Subscriber Implementation
AnalyticsSubscriber::AnalyticsSubscriber() : totalMessages_(0) {
    std::cout << "Analytics Subscriber initialized" << std::endl;
}

void AnalyticsSubscriber::onMarketData(const MarketData& data) {
    std::lock_guard<std::mutex> lock(dataMutex_);
    
    priceData_[data.symbol].push_back(data.price);
    volumeData_[data.symbol].push_back(data.size);
    totalMessages_++;
    
    calculateStatistics(data);
}

void AnalyticsSubscriber::calculateStatistics(const MarketData& data) {
    auto& prices = priceData_[data.symbol];
    auto& volumes = volumeData_[data.symbol];
    
    if (prices.size() % 100 == 0) { // Log every 100 messages
        double avgPrice = std::accumulate(prices.begin(), prices.end(), 0.0) / prices.size();
        int totalVolume = std::accumulate(volumes.begin(), volumes.end(), 0);
        
        std::cout << "Analytics: " << data.symbol 
                  << " Avg Price: " << avgPrice 
                  << " Total Volume: " << totalVolume 
                  << " Messages: " << prices.size() << std::endl;
    }
}

void AnalyticsSubscriber::generateReports() {
    std::lock_guard<std::mutex> lock(dataMutex_);
    
    std::cout << "\n=== ANALYTICS REPORT ===" << std::endl;
    std::cout << "Total Messages Processed: " << totalMessages_ << std::endl;
    
    for (const auto& [symbol, prices] : priceData_) {
        if (!prices.empty()) {
            double avgPrice = std::accumulate(prices.begin(), prices.end(), 0.0) / prices.size();
            auto minmax = std::minmax_element(prices.begin(), prices.end());
            
            std::cout << symbol << ": Avg=" << avgPrice 
                      << " Min=" << *minmax.first 
                      << " Max=" << *minmax.second 
                      << " Count=" << prices.size() << std::endl;
        }
    }
    std::cout << "========================\n" << std::endl;
}

size_t AnalyticsSubscriber::getTotalMessages() const {
    return totalMessages_;
}

double AnalyticsSubscriber::getAveragePrice(const std::string& symbol) const {
    std::lock_guard<std::mutex> lock(const_cast<std::mutex&>(dataMutex_));
    
    auto it = priceData_.find(symbol);
    if (it != priceData_.end() && !it->second.empty()) {
        return std::accumulate(it->second.begin(), it->second.end(), 0.0) / it->second.size();
    }
    return 0.0;
}

int AnalyticsSubscriber::getTotalVolume(const std::string& symbol) const {
    std::lock_guard<std::mutex> lock(const_cast<std::mutex&>(dataMutex_));
    
    auto it = volumeData_.find(symbol);
    if (it != volumeData_.end()) {
        return std::accumulate(it->second.begin(), it->second.end(), 0);
    }
    return 0;
}
