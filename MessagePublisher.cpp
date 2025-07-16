#include "MessagePublisher.h"
#include <iostream>

void MessagePublisher::publish(const std::string& msg) {
    std::cout << "Publishing: " << msg << std::endl;
}