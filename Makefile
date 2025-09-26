CXX = g++
CXXFLAGS = -std=c++17 -Wall -O2 -pthread

all: main

main: main.cpp FeedHandler.cpp MessagePublisher.cpp ThreadSafeMessageBroker.cpp Subscribers.cpp
	$(CXX) $(CXXFLAGS) $^ -o feedhandler

clean:
	rm -f feedhandler

test: main
	@echo "Starting feed handler test..."
	@echo "Note: This requires a market data server running on localhost:9000"
	@echo "You can simulate one with: nc -l 9000 < test_data.txt"
	./feedhandler