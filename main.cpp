#include <iostream>
#include "FeedHandler.h"

int main() {
    FeedHandler handler("127.0.0.1", 9000); // Connect to local test server
    handler.start();
    return 0;
} 