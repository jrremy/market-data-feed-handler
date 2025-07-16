# Market Data Feed Handler (C++)

## Overview
This project is a simple real-time market data feed handler written in C++. It connects to a TCP server, receives market data messages (in CSV format), parses them, and prints the parsed data. This is a foundational component for quantitative trading systems.

## Features
- Connects to a TCP server (e.g., localhost:9000)
- Receives and processes real-time messages
- Parses CSV messages (e.g., `SYMBOL,PRICE,SIZE`)
- Prints parsed data to the console

## Build
Ensure you have a C++17-compatible compiler (e.g., g++ or clang++).

```
make
```
This will produce an executable named `feedhandler`.

## Run
Start a test TCP server in one terminal:
```
nc -l 9000
```

In another terminal, run the feed handler:
```
./feedhandler
```

Type messages like the following into the netcat terminal:
```
AAPL,150.23,100
GOOG,2800.50,50
```

You should see the received (and later, parsed) messages printed in the feed handler terminal.

## Next Steps
- Parse and process messages
- Store or publish parsed data
- Add error handling and metrics

## License
MIT
