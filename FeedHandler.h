#pragma once
#include <string>

class FeedHandler {
public:
    FeedHandler(const std::string& host, int port);
    void start();
private:
    std::string host_;
    int port_;
    int sockfd_;
    void processMessage(const std::string& msg);
};