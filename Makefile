CXX = g++
CXXFLAGS = -std=c++17 -Wall

all: main

main: main.cpp FeedHandler.cpp MessagePublisher.cpp
	$(CXX) $(CXXFLAGS) $^ -o feedhandler

clean:
	rm -f feedhandler