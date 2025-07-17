#pragma once
#include <string>

class FeedHandler {
public:
    FeedHandler(const std::string& host, int port);
    void start();
    void processMessage(const std::string& msg);
private:
    struct MarketData {
        std::string symbol;
        double price;
        int size;
    };
    std::string host_;
    int port_;
    int sockfd_;
};