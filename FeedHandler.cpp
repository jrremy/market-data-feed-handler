#include "FeedHandler.h"
#include <iostream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <sstream>

FeedHandler::FeedHandler(const std::string& host, int port)
    : host_(host), port_(port), sockfd_(-1) {}

void FeedHandler::start() {
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
        return;
    }
    char buffer[1024];
    while (true) {
        ssize_t n = read(sockfd_, buffer, sizeof(buffer) - 1);
        if (n <= 0) break;
        buffer[n] = '\0';
        processMessage(std::string(buffer));
    }
    close(sockfd_);
}

void FeedHandler::processMessage(const std::string& msg) {
    std::istringstream ss(msg);
    std::string symbol, price_str, size_str;

    if (std::getline(ss, symbol, ',') &&
        std::getline(ss, price_str, ',') &&
        std::getline(ss, size_str)) {
        try {
            double price = std::stod(price_str);
            int size = std::stoi(size_str);
            MarketData data{symbol, price, size};
            std::cout << "Parsed: Symbol=" << data.symbol
                      << ", Price=" << data.price
                      << ", Size=" << data.size << std::endl;
        } catch (const std::exception& e) {
            std::cerr << "Parse error: " << e.what() << " in message: " << msg << std::endl;
        }
    } else {
        std::cerr << "Malformed message: " << msg << std::endl;
    }
}