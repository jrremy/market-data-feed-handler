#!/usr/bin/env python3
"""
Market Data Generator
Simulates a real market data feed for testing the feed handler.
Sends fake market data messages over TCP socket at configurable rates.
"""

import socket
import time
import random
import argparse
from datetime import datetime


class MarketDataGenerator:
    def __init__(self, host: str = "127.0.0.1", port: int = 9000):
        self.host = host
        self.port = port
        self.socket = None
        self.sequence_number = 0
        self.symbols = ["AAPL", "GOOGL", "MSFT", "TSLA", "AMZN", "META", "NVDA", "NFLX"]
        self.base_prices = {
            "AAPL": 150.0,
            "GOOGL": 2800.0,
            "MSFT": 350.0,
            "TSLA": 200.0,
            "AMZN": 3000.0,
            "META": 300.0,
            "NVDA": 400.0,
            "NFLX": 400.0,
        }

    def connect(self) -> bool:
        """Connect to the feed handler server."""
        try:
            self.socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            self.socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
            self.socket.connect((self.host, self.port))
            print(f"Connected to {self.host}:{self.port}")
            return True
        except Exception as e:
            print(f"Failed to connect: {e}")
            return False

    def disconnect(self):
        """Disconnect from the server."""
        if self.socket:
            self.socket.close()
            self.socket = None
            print("Disconnected")

    def generate_market_data(self) -> str:
        """Generate a single market data message."""
        symbol = random.choice(self.symbols)
        base_price = self.base_prices[symbol]

        # Add some realistic price movement (-2% to +2%)
        price_change = random.uniform(-0.02, 0.02)
        price = base_price * (1 + price_change)

        # Generate realistic volume (100-5000 shares)
        volume = random.randint(100, 5000)

        # Current timestamp
        timestamp = datetime.now().strftime("%Y-%m-%dT%H:%M:%S.%fZ")

        # Format: symbol,price,volume,timestamp,sequence_number
        message = f"{symbol},{price:.2f},{volume},{timestamp},{self.sequence_number}\n"
        self.sequence_number += 1

        return message

    def send_message(self, message: str) -> bool:
        """Send a message to the server."""
        try:
            self.socket.sendall(message.encode("utf-8"))
            return True
        except Exception as e:
            print(f"Failed to send message: {e}")
            return False

    def run_continuous(self, messages_per_second: int, duration_seconds: int = None):
        """Run the generator continuously at specified rate."""
        if not self.connect():
            return

        print(f"Generating {messages_per_second} messages/second")
        if duration_seconds:
            print(f"Duration: {duration_seconds} seconds")
        else:
            print("Duration: infinite (Ctrl+C to stop)")

        # Calculate sleep time between messages
        sleep_time = 1.0 / messages_per_second

        start_time = time.time()
        messages_sent = 0

        try:
            while True:
                message = self.generate_market_data()

                if not self.send_message(message):
                    print("Connection lost, attempting to reconnect...")
                    self.disconnect()
                    if not self.connect():
                        print("Reconnection failed, exiting")
                        break
                    continue

                messages_sent += 1

                # Print progress every 1000 messages
                if messages_sent % 1000 == 0:
                    elapsed = time.time() - start_time
                    actual_rate = messages_sent / elapsed if elapsed > 0 else 0
                    print(
                        f"Sent {messages_sent} messages, rate: {actual_rate:.1f} msg/sec"
                    )

                # Check duration limit
                if duration_seconds and (time.time() - start_time) >= duration_seconds:
                    break

                time.sleep(sleep_time)

        except KeyboardInterrupt:
            print("\nStopping generator...")

        finally:
            elapsed = time.time() - start_time
            actual_rate = messages_sent / elapsed if elapsed > 0 else 0
            print(f"\nFinal stats:")
            print(f"Messages sent: {messages_sent}")
            print(f"Duration: {elapsed:.2f} seconds")
            print(f"Average rate: {actual_rate:.1f} msg/sec")
            self.disconnect()

    def run_burst(self, total_messages: int, burst_rate: int = 10000):
        """Send a burst of messages at high rate."""
        if not self.connect():
            return

        print(
            f"Sending {total_messages} messages in burst mode at {burst_rate} msg/sec"
        )

        sleep_time = 1.0 / burst_rate
        start_time = time.time()

        try:
            for i in range(total_messages):
                message = self.generate_market_data()

                if not self.send_message(message):
                    print(f"Failed to send message {i + 1}")
                    break

                if (i + 1) % 1000 == 0:
                    print(f"Sent {i + 1}/{total_messages} messages")

                time.sleep(sleep_time)

        except KeyboardInterrupt:
            print("\nBurst interrupted")

        finally:
            elapsed = time.time() - start_time
            actual_rate = total_messages / elapsed if elapsed > 0 else 0
            print(f"\nBurst complete:")
            print(f"Messages sent: {total_messages}")
            print(f"Duration: {elapsed:.2f} seconds")
            print(f"Rate: {actual_rate:.1f} msg/sec")
            self.disconnect()


def main():
    parser = argparse.ArgumentParser(description="Market Data Generator")
    parser.add_argument(
        "--host", default="127.0.0.1", help="Server host (default: 127.0.0.1)"
    )
    parser.add_argument(
        "--port", type=int, default=9000, help="Server port (default: 9000)"
    )
    parser.add_argument(
        "--rate", type=int, default=1000, help="Messages per second (default: 1000)"
    )
    parser.add_argument(
        "--duration", type=int, help="Duration in seconds (default: infinite)"
    )
    parser.add_argument(
        "--burst", type=int, help="Send burst of N messages at high rate"
    )
    parser.add_argument(
        "--burst-rate", type=int, default=10000, help="Burst rate (default: 10000)"
    )

    args = parser.parse_args()

    generator = MarketDataGenerator(args.host, args.port)

    if args.burst:
        generator.run_burst(args.burst, args.burst_rate)
    else:
        generator.run_continuous(args.rate, args.duration)


if __name__ == "__main__":
    main()
