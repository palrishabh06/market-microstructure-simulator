# Market Microstructure Exchange Simulator

A C++20 simulation of a financial exchange matching engine and limit order book, designed to model core market microstructure concepts such as price-time priority, order matching, partial fills, order cancellation, order modification, and trade execution.

The project is built from scratch using modern C++ data structures and provides a foundation for studying exchange infrastructure, algorithmic trading systems, market data processing, and high-frequency trading concepts.

---

## Overview

A financial exchange receives buy and sell orders from market participants and determines which orders can be executed.

At the center of this process is the **Limit Order Book (LOB)**.

The order book maintains:

- Buy orders (Bids)
- Sell orders (Asks)
- Price levels
- Queue priority
- Remaining order quantities

The **Matching Engine** compares incoming orders against the best available orders on the opposite side of the book and generates trades when execution conditions are satisfied.

This project implements that core mechanism using **C++20**.

---

## Key Features

- Limit Order Book implementation
- Price-time priority matching
- Limit order support
- Market order matching
- Partial order fills
- Multiple executions from a single incoming order
- Trade generation
- Order cancellation
- Order quantity modification
- Best bid tracking
- Best ask tracking
- Bid-ask spread calculation
- Mid-price calculation
- Quantity available at a price level
- Price-level tracking
- Matching engine abstraction
- Trade history
- C++20 implementation
- CMake build system
- Unit testing
- Linux/macOS development environment
- Extensible architecture for future market-data and performance modules

---

# Market Microstructure

## Limit Order Book

The exchange maintains two sides of the order book:

1. **Bid side** — BUY orders
2. **Ask side** — SELL orders

The highest bid and lowest ask represent the best available prices in the market.

---

## Bid Side

The bid side contains BUY orders.

Higher bid prices receive higher priority.

### Example

```text
BUY ORDERS

Price      Quantity
-------------------
105.00        100
104.50         75
104.00        200
```

The best bid is:

```text
105.00
```

---

## Ask Side

The ask side contains SELL orders.

Lower ask prices receive higher priority.

### Example

```text
SELL ORDERS

Price      Quantity
-------------------
105.50         80
106.00        150
106.50        100
```

The best ask is:

```text
105.50
```

---

## Price-Time Priority

The matching engine follows the **price-time priority** principle.

For BUY orders:

```text
Higher price → Higher priority
```

For SELL orders:

```text
Lower price → Higher priority
```

When multiple orders have the same price, the order that arrived first is executed first.

### Example

```text
BUY Order #1 → 100 units @ 105.00
BUY Order #2 → 200 units @ 105.00
BUY Order #3 → 150 units @ 105.00
```

All three orders have the same price.

Therefore:

```text
Order #1 → First priority
Order #2 → Second priority
Order #3 → Third priority
```

The implementation uses:

- `std::map` for ordered price levels
- `std::deque` for FIFO ordering within each price level

---

# Order Types

## Limit Orders

A limit order specifies the maximum price a buyer is willing to pay or the minimum price a seller is willing to accept.

### Example

Existing SELL order:

```text
SELL
Price:    106.00
Quantity: 100
```

Incoming BUY order:

```text
BUY
Price:    105.00
Quantity: 50
```

The orders cannot execute because:

```text
105.00 < 106.00
```

If the BUY order instead arrives at:

```text
107.00
```

then it can match the SELL order at:

```text
106.00
```

The execution occurs at the **resting order's price**.

---

## Market Orders

A market order attempts to execute immediately against available liquidity on the opposite side of the order book.

A market order:

- Does not specify an execution price
- Matches against the best available prices
- Can consume liquidity across multiple price levels
- Does not rest in the order book after matching

---

# Matching Engine

The `MatchingEngine` receives incoming orders and sends them through the matching process.

The general execution flow is:

```text
Incoming Order
      |
      v
Matching Engine
      |
      v
Check Opposite Side
      |
      v
Best Available Price
      |
      v
Price-Time Priority
      |
      v
Execute Trade
      |
      v
Update Quantities
      |
      v
Remove Filled Orders
      |
      v
Rest Remaining Limit Quantity
```

The matching process continues until:

- The incoming order is completely filled
- There is no compatible liquidity
- A limit order reaches a price boundary

---

# Partial Fills

The simulator supports partial order execution.

A partial fill occurs when the incoming order quantity is larger than the available quantity of a resting order.

### Example

Existing SELL order:

```text
SELL #100
Price:    105.00
Quantity: 100
```

Incoming BUY order:

```text
BUY #200
Price:    106.00
Quantity: 40
```

The orders can execute because:

```text
106.00 >= 105.00
```

The resulting trade is:

```text
Price:    105.00
Quantity: 40
```

The remaining SELL quantity becomes:

```text
60
```

The SELL order remains active in the order book.

---

# Multiple Price-Level Matching

A single incoming order can consume liquidity across multiple price levels.

### Example

Existing ask book:

```text
ASK BOOK

Price      Quantity
-------------------
105.00         50
106.00         75
107.00        100
```

Incoming market BUY:

```text
Quantity: 150
```

The matching engine consumes liquidity from the best ask upward:

```text
50  @ 105.00
75  @ 106.00
25  @ 107.00
```

Total executed quantity:

```text
150 units
```

This models basic liquidity consumption across multiple price levels.

---

# Trade Generation

Every successful execution produces a `Trade` object.

A trade contains:

- Incoming order ID
- Resting order ID
- Execution price
- Execution quantity
- Execution timestamp

### Example

```text
Incoming Order ID : 200
Resting Order ID  : 100
Price             : 105.00
Quantity          : 40
Timestamp         : ...
```

The matching engine stores executed trades in its trade history.

---

# Order Management

## Add Order

Limit orders can be inserted into the appropriate side of the order book.

```cpp
orderBook.addLimitOrder(order);
```

---

## Cancel Order

An active order can be cancelled using its unique order ID.

```cpp
orderBook.cancelOrder(orderId);
```

The order is removed from its price-level queue.

If the removal leaves the price level empty, the price level is also removed.

---

## Modify Order

The remaining quantity of an active order can be modified.

```cpp
orderBook.modifyOrder(orderId, newQuantity);
```

If the new quantity is zero, the order is removed from the order book.

---

# Market Data Metrics

The order book exposes several important market microstructure metrics.

## Best Bid

The best bid is the highest available BUY price.

```cpp
double bid = orderBook.bestBid();
```

---

## Best Ask

The best ask is the lowest available SELL price.

```cpp
double ask = orderBook.bestAsk();
```

---

## Bid-Ask Spread

The bid-ask spread is calculated as:

```text
Spread = Best Ask - Best Bid
```

### Example

```text
Best Bid = 105.00
Best Ask = 105.50

Spread = 0.50
```

Available through:

```cpp
double spread = orderBook.spread();
```

---

## Mid Price

The mid price is calculated as:

```text
Mid Price = (Best Bid + Best Ask) / 2
```

### Example

```text
Best Bid = 105.00
Best Ask = 105.50

Mid Price = 105.25
```

Available through:

```cpp
double mid = orderBook.midPrice();
```

---

## Quantity at Price

The simulator can determine the total quantity available at a specific price level.

```cpp
std::uint64_t quantity =
    orderBook.quantityAtPrice(Side::BUY, 105.00);
```

---

## Price-Level Count

The number of active price levels can be queried for either side.

```cpp
std::size_t bidLevels =
    orderBook.priceLevelCount(Side::BUY);

std::size_t askLevels =
    orderBook.priceLevelCount(Side::SELL);
```

---

# Data Structures

The core order book uses:

```cpp
std::map
std::deque
```

## Bid Book

The bid book is stored using a descending price ordering:

```cpp
std::map<
    double,
    std::deque<Order>,
    std::greater<double>
>
```

This keeps the highest bid at the beginning of the map.

---

## Ask Book

The ask book uses the default ascending ordering:

```cpp
std::map<
    double,
    std::deque<Order>
>
```

This keeps the lowest ask at the beginning of the map.

---

## Why These Data Structures?

### `std::map`

Used to maintain sorted price levels.

This allows the matching engine to efficiently access:

- Highest bid
- Lowest ask
- Individual price levels

### `std::deque`

Used to maintain FIFO ordering among orders at the same price.

This naturally represents the time-priority queue required by price-time matching.

Together, these structures provide a simple and transparent representation of a limit order book.

---

# Order Lifecycle

A typical order follows this lifecycle:

```text
              +----------------+
              | Incoming Order |
              +-------+--------+
                      |
                      v
              +---------------+
              | Matching      |
              | Engine        |
              +-------+-------+
                      |
             +--------+--------+
             |                 |
          Match             No Match
             |                 |
             v                 v
        Execute Trade      Rest Limit
             |                 |
             v                 v
       Update Quantity    Order Book
             |
       +-----+-----+
       |           |
    Filled      Partially Filled
       |           |
       v           v
    Removed    Remains Active
```

---

# Project Architecture

```text
market-microstructure-exchange-simulator/
│
├── include/
│   ├── order_book/
│   │   ├── Order.h
│   │   └── OrderBook.h
│   │
│   └── matching_engine/
│       ├── MatchingEngine.h
│       └── Trade.h
│
├── src/
│   ├── order_book/
│   │   └── OrderBook.cpp
│   │
│   └── matching_engine/
│       └── MatchingEngine.cpp
│
├── tests/
│   ├── test_order_book.cpp
│   └── test_matching_engine.cpp
│
├── benchmarks/
│
├── data/
│
├── python/
│
├── CMakeLists.txt
├── README.md
└── .gitignore
```

---

# Component Responsibilities

## `Order`

Defines the structure of an order.

An order contains:

- Unique order ID
- Side
- Order type
- Price
- Quantity
- Timestamp

---

## `Trade`

Represents an executed transaction.

A trade contains:

- Incoming order ID
- Resting order ID
- Execution price
- Execution quantity
- Timestamp

---

## `OrderBook`

The `OrderBook` is responsible for:

- Maintaining bids
- Maintaining asks
- Adding limit orders
- Cancelling orders
- Modifying orders
- Matching incoming orders
- Calculating market metrics

---

## `MatchingEngine`

The `MatchingEngine` is responsible for:

- Receiving incoming orders
- Determining the matching side
- Executing matching logic
- Recording generated trades
- Resting remaining limit-order quantity

---

# Complexity

For the current implementation:

| Operation | Approximate Complexity |
|-----------|------------------------|
| Add order | O(log P) |
| Best bid | O(1) |
| Best ask | O(1) |
| Spread | O(1) |
| Mid price | O(1) |
| Quantity at price | O(Q) |
| Cancel order | O(N) |
| Modify order | O(N) |
| Matching | O(K log P) |

Where:

- `P` = number of active price levels
- `Q` = number of orders at a specific price level
- `N` = number of active orders
- `K` = number of orders/price levels consumed during matching

The current implementation prioritizes correctness, deterministic behavior, and clarity.

Future versions can optimize order lookup and cancellation using dedicated order-ID indexing.

---

# Testing

The project contains unit tests covering the core behavior of the exchange simulator.

Current test coverage includes:

- Limit order insertion
- Bid-side ordering
- Ask-side ordering
- Best bid
- Best ask
- Bid-ask spread
- Mid price
- Quantity tracking
- Order cancellation
- Order modification
- Order matching
- Partial fills
- Price-time priority
- Trade generation
- Matching engine integration

---

## Build Tests

Configure the project:

```bash
cmake -S . -B build
```

Build the project:

```bash
cmake --build build
```

Run the order book tests:

```bash
./build/test_order_book
```

Expected output:

```text
OrderBook tests passed!
```

---

# Example

A simple matching scenario can be created using the `MatchingEngine`.

```cpp
#include "matching_engine/MatchingEngine.h"

int main() {

    MatchingEngine engine;

    Order sellOrder{
        1,
        Side::SELL,
        OrderType::LIMIT,
        105.0,
        100,
        1
    };

    Order buyOrder{
        2,
        Side::BUY,
        OrderType::LIMIT,
        106.0,
        40,
        2
    };

    engine.submitOrder(sellOrder);
    engine.submitOrder(buyOrder);

    return 0;
}
```

The BUY order can execute against the SELL order because:

```text
BUY price 106.00 >= SELL price 105.00
```

The resulting trade is:

```text
Price:    105.00
Quantity: 40
```

The remaining SELL quantity is:

```text
60
```

---

# Example Order Book

Consider the following book:

```text
                 ORDER BOOK

        BIDS                    ASKS
Price       Qty             Price       Qty
------     -----            ------     -----
105.00      100             105.50       80
104.50       75             106.00      150
104.00      200             106.50      100
```

The market state is:

```text
Best Bid : 105.00
Best Ask : 105.50
Spread   : 0.50
Mid      : 105.25
```

This information can be used as the basis for further market microstructure analysis.

---

# Design Principles

The project is designed around several important systems principles.

## Deterministic Matching

Given the same sequence of orders, the matching engine should produce the same sequence of executions.

This makes the simulator suitable for reproducible testing and future historical market-data replay.

---

## Separation of Responsibilities

The order book manages market state and order queues.

The matching engine manages order submission, matching coordination, and trade recording.

This separation makes the architecture easier to extend and test.

---

## Explicit Data Structures

Standard library containers are used to represent price levels and FIFO order queues directly.

This makes the relationship between the data structures and the market microstructure model easy to understand.

---

## Testability

Core exchange functionality is separated into components that can be tested independently.

The project contains dedicated tests for order-book and matching-engine behavior.

---

## Extensibility

The current architecture is designed to support future components such as:

- Market-data replay
- Order-flow generation
- Strategy simulation
- Backtesting
- Performance benchmarking
- Exchange latency simulation
- Transaction costs
- Slippage analysis
- PnL calculation

---

# Planned Improvements

The project is designed for continued development toward a more complete exchange and market-data simulation framework.

## Order Book Optimization

Planned improvements include:

- Faster order lookup
- Order-ID indexing
- Reduced cancellation complexity
- More cache-friendly data structures
- Integer price ticks instead of floating-point prices

---

## Market Data

Future market-data functionality may include:

- Level-1 market data
- Level-2 order book snapshots
- Trade feeds
- Order-flow statistics
- Historical market-data replay

---

## Exchange Simulation

Future exchange infrastructure components may include:

- Exchange gateway simulation
- Matching latency
- Network latency
- Order-processing latency
- Event queues
- Deterministic event replay

---

## Trading Research

Future research functionality may include:

- Trading strategy simulation
- Backtesting
- PnL calculation
- Transaction costs
- Slippage analysis
- Market impact
- Execution-quality analysis

---

## Performance Benchmarking

Future benchmarks will measure:

- Orders processed per second
- Matching latency
- Order insertion latency
- Cancellation latency
- Memory usage
- Performance under deep order books
- Performance under high order flow

---

# Technology Stack

## Core

- C++20
- Standard Template Library (STL)
- CMake

## Data Structures

- `std::map`
- `std::deque`
- `std::vector`

## Development

- Git
- GDB
- VS Code
- Linux/macOS

## Testing

- C++ assertions
- Custom unit tests
- CMake build system

---

# Development Workflow

The project follows a simple development workflow:

```text
Design
  ↓
Implement
  ↓
Build
  ↓
Run Tests
  ↓
Debug
  ↓
Benchmark
  ↓
Optimize
```

The architecture is intentionally modular so that individual components can be improved without redesigning the entire system.

---

# Future Performance Engineering

The current implementation uses standard library containers for clarity and correctness.

For a production-oriented low-latency exchange simulator, additional optimization would be required.

Potential optimization areas include:

- Cache locality
- Memory allocation
- Object lifetime management
- Order-ID lookup
- Price representation
- Branch prediction
- Data-oriented design
- Lock-free structures where appropriate
- CPU affinity
- Latency measurement
- Throughput benchmarking

These improvements are part of the future development roadmap.

---

# Future Market-Data Pipeline

A future version can extend the project into a complete market-data research pipeline:

```text
Historical Market Data
          |
          v
     Data Parser
          |
          v
    Event Replay
          |
          v
    Matching Engine
          |
          v
     Order Book
          |
          v
   Market Data Feed
          |
          v
   Python Analytics
          |
          v
 Strategy / Research
```

This would allow historical order events to be replayed through the matching engine and analyzed using Python-based statistical and quantitative tools.

---

# Future Trading Strategy Layer

The matching engine can eventually serve as the execution layer for simulated trading strategies.

A potential architecture is:

```text
Market Data
     |
     v
Strategy
     |
     v
Order Generation
     |
     v
Matching Engine
     |
     v
Executed Trades
     |
     v
Portfolio / PnL
     |
     v
Performance Analysis
```

This would allow strategies to be evaluated using realistic order-book interactions rather than simple historical price series.

---

# Project Goals

The long-term goal is to develop the project from a basic order-book simulator into a modular research environment covering:

1. Exchange matching
2. Market microstructure
3. Market-data processing
4. Order-flow simulation
5. Strategy backtesting
6. Execution analysis
7. Performance benchmarking
8. Quantitative research

---

# Why This Project?

Exchange matching engines are latency-sensitive systems where correctness, deterministic behavior, data structures, and performance are critical.

This project provides hands-on implementation of concepts fundamental to:

- Algorithmic Trading
- High-Frequency Trading
- Market Microstructure
- Exchange Infrastructure
- Financial Systems
- Low-Latency Systems
- Quantitative Research
- Performance Engineering

Instead of treating a trading system as a black box, this project implements the core matching process from the ground up.

---

# Current Status

The core limit order book and matching engine are implemented and tested.

Current functionality includes:

- [x] Limit orders
- [x] Price-time priority
- [x] Partial fills
- [x] Multi-level matching
- [x] Trade generation
- [x] Order cancellation
- [x] Order modification
- [x] Best bid
- [x] Best ask
- [x] Bid-ask spread
- [x] Mid price
- [x] Quantity-at-price
- [x] Price-level tracking
- [x] Matching engine
- [x] Trade history
- [x] Unit tests
- [x] CMake build system

The project is being extended toward a more complete exchange and market-data simulation framework.

---

# Limitations

The current implementation is a research and educational simulator rather than a production exchange engine.

Current limitations include:

- Order cancellation uses a linear search
- Order modification uses a linear search
- Prices are represented using `double`
- No persistent market-data feed
- No historical market-data replay
- No network layer
- No real exchange connectivity
- No production-grade latency model
- No complete portfolio accounting
- No production-grade risk management system

These limitations provide clear opportunities for future optimization and extension.

---

# Build Instructions

## Requirements

You need:

- C++20 compatible compiler
- CMake 3.20 or newer
- Git

Recommended development environments:

- Linux
- macOS
- Other Unix-like systems with a C++20 compiler

---

## Clone the Repository

```bash
git clone https://github.com/palrishabh06/market-microstructure-simulator.git
cd market-microstructure-simulator
```

---

## Configure the Project

```bash
cmake -S . -B build
```

---

## Build

```bash
cmake --build build
```

---

## Run Tests

```bash
./build/test_order_book
```

---

# Repository Structure

```text
market-microstructure-exchange-simulator/
│
├── include/
│   ├── order_book/
│   │   ├── Order.h
│   │   └── OrderBook.h
│   │
│   └── matching_engine/
│       ├── MatchingEngine.h
│       └── Trade.h
│
├── src/
│   ├── order_book/
│   │   └── OrderBook.cpp
│   │
│   └── matching_engine/
│       └── MatchingEngine.cpp
│
├── tests/
│   ├── test_order_book.cpp
│   └── test_matching_engine.cpp
│
├── benchmarks/
├── data/
├── python/
│
├── CMakeLists.txt
├── README.md
└── .gitignore
```

---

# Contributing

Contributions and improvements are welcome.

Potential areas for contribution include:

- Performance optimization
- Additional unit tests
- Benchmarking infrastructure
- Market-data replay
- Order-flow simulation
- Better order indexing
- Additional order types
- Trading strategy simulation
- Documentation
- Quantitative analytics

A typical contribution workflow is:

```bash
git checkout -b feature-name
```

Make your changes, build the project, and run the tests.

Then commit:

```bash
git add .
git commit -m "Describe your change"
```

Push the branch:

```bash
git push origin feature-name
```

---

# Disclaimer

This project is an educational and research-oriented exchange simulator.

It is not connected to any real financial exchange and should not be used for live trading or financial decision-making.

---

# Author

**Rishabh Raj**

Computer Science & Engineering  
Birla Institute of Technology, Mesra

- GitHub: [palrishabh06](https://github.com/palrishabh06)
- LinkedIn: [Rishabh Raj](https://www.linkedin.com/in/palrishabh06)

---

# Project Repository

[Market Microstructure Exchange Simulator](https://github.com/palrishabh06/market-microstructure-simulator)

---

## Project Status

**Active Development**

The core exchange matching infrastructure is implemented, tested, and structured for future development in market microstructure research, quantitative trading systems, and low-latency performance engineering.
