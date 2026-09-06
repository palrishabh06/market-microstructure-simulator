# Market Microstructure Exchange Simulator

A high-performance C++20 simulation of a financial exchange matching engine and limit order book, designed to model core market microstructure concepts such as price-time priority, order matching, partial fills, order cancellation, order modification, and trade execution.

The project is built from scratch using modern C++ data structures and is intended as a foundation for studying exchange infrastructure, algorithmic trading systems, market data processing, and high-frequency trading (HFT) concepts.

---

## Overview

A financial exchange receives buy and sell orders from market participants and determines which orders can be executed.

At the center of this process is the **Limit Order Book (LOB)**.

The order book maintains:

- Buy orders (Bids)
- Sell orders (Asks)
- Price levels
- Queue priority within each price level
- Remaining order quantities

The **Matching Engine** continuously compares incoming orders against the best available orders on the opposite side of the book.

This project implements that core mechanism in C++20.

---

## Key Features

- Limit Order Book implementation
- Price-time priority matching
- Limit order support
- Market order matching
- Partial order fills
- Multiple executions from a single order
- Trade generation
- Order cancellation
- Order quantity modification
- Best bid / best ask tracking
- Bid-ask spread calculation
- Mid-price calculation
- Quantity available at a price level
- Price-level tracking
- Matching engine abstraction
- Trade history
- C++20 implementation
- CMake build system
- Unit testing
- Linux/macOS compatible development environment
- Designed for future performance benchmarking and market-data simulation

---

# Market Microstructure

## Limit Order Book

The exchange maintains two sides of the order book.

### Bid Side

Contains BUY orders.

The highest price has the highest priority.

Example:

```text
BUY ORDERS

Price      Quantity
-------------------
105.00        100
104.50         75
104.00        200

The best bid is:

105.00
Ask Side

Contains SELL orders.

The lowest price has the highest priority.

Example:

SELL ORDERS

Price      Quantity
-------------------
105.50         80
106.00        150
106.50        100

The best ask is:

105.50
Price-Time Priority

The matching engine follows the standard price-time priority principle.

For BUY orders:

Higher price → higher priority

For SELL orders:

Lower price → higher priority

If multiple orders have the same price, the order that arrived first is executed first.

Example:

BUY Order #1 → 100 @ 105.00
BUY Order #2 → 200 @ 105.00
BUY Order #3 → 150 @ 105.00

Order #1 has priority over #2, and #2 has priority over #3.

This is implemented using:

std::map
std::deque

The map maintains price-level ordering while the deque maintains FIFO ordering within each price level.

Order Types
Limit Orders

A limit order specifies the maximum price a buyer is willing to pay or the minimum price a seller is willing to accept.

Example:

BUY  100 units @ 105.00
SELL 100 units @ 106.00

These orders cannot execute because:

105.00 < 106.00

If a BUY order arrives at:

107.00

it can match the SELL order at:

106.00

The execution price is the resting order's price.

Market Orders

A market order attempts to execute immediately against available liquidity on the opposite side of the book.

A market order:

Does not specify an execution price
Executes against the best available prices
Can consume multiple price levels
Does not rest in the order book after matching
Matching Engine

The MatchingEngine receives incoming orders and sends them to the order book for execution.

The basic flow is:

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
Partial Fills

The simulator supports partial execution.

Example:

Existing SELL order:

SELL #100
Price:    105.00
Quantity: 100

Incoming BUY order:

BUY #200
Price:    106.00
Quantity: 40

Execution:

Trade:
Price:    105.00
Quantity: 40

Remaining SELL quantity:

60

The SELL order remains active in the book.

Multiple Price-Level Matching

A single incoming order can consume liquidity across multiple price levels.

Example:

ASK BOOK

Price      Quantity
-------------------
105.00        50
106.00        75
107.00       100

Incoming market BUY:

Quantity: 150

Execution:

50  @ 105.00
75  @ 106.00
25  @ 107.00

Total executed:

150 units

This allows the simulator to model basic market impact and liquidity consumption.

Trade Generation

Every successful execution produces a Trade object containing:

Incoming order ID
Resting order ID
Execution price
Execution quantity
Execution timestamp

Example:

Incoming Order ID : 200
Resting Order ID  : 100
Price             : 105.00
Quantity          : 40
Timestamp         : ...

The matching engine stores executed trades in its trade history.

Order Management
Add Order

Limit orders can be inserted into the order book.

orderBook.addLimitOrder(order);
Cancel Order

An active order can be cancelled using its unique order ID.

orderBook.cancelOrder(orderId);

The operation removes the order from its price-level queue.

Modify Order

The remaining quantity of an active order can be modified.

orderBook.modifyOrder(orderId, newQuantity);

If the new quantity is zero, the order is removed.

Market Data Metrics

The order book exposes several important market microstructure metrics.

Best Bid

Highest available BUY price.

double bid = orderBook.bestBid();
Best Ask

Lowest available SELL price.

double ask = orderBook.bestAsk();
Bid-Ask Spread
Spread = Best Ask - Best Bid

Example:

Best Bid = 105.00
Best Ask = 105.50

Spread = 0.50

Available through:

double spread = orderBook.spread();
Mid Price
Mid Price = (Best Bid + Best Ask) / 2

Available through:

double mid = orderBook.midPrice();
Quantity at Price

The simulator can determine how much liquidity is available at a specific price.

orderBook.quantityAtPrice(
    Side::BUY,
    105.00
);
Price-Level Count

The number of active price levels can be queried for either side.

orderBook.priceLevelCount(Side::BUY);
orderBook.priceLevelCount(Side::SELL);
Data Structures

The core order book uses:

std::map
std::deque
Bid Book
std::map<
    double,
    std::deque<Order>,
    std::greater<double>
>

This keeps the highest bid at the beginning.

Ask Book
std::map<
    double,
    std::deque<Order>
>

This keeps the lowest ask at the beginning.

Why These Structures?

std::map provides ordered price levels.

std::deque provides efficient FIFO queue behavior for orders at the same price.

Together they naturally represent a price-time-priority limit order book.

Order Lifecycle

A typical order follows this lifecycle:

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
Project Architecture
market-microstructure-exchange-simulator/
│
├── include/
│   ├── order_book/
│   │   ├── Order.h
│   │   └── OrderBook.h
│   │
│   ├── matching_engine/
│   │   ├── MatchingEngine.h
│   │   └── Trade.h
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
Component Responsibilities
Order

Defines the structure of an order.

Contains:

Order ID
Side
Order type
Price
Quantity
Timestamp
Trade

Represents an executed transaction.

Contains:

Incoming order ID
Resting order ID
Execution price
Execution quantity
Timestamp
OrderBook

Responsible for:

Maintaining bids
Maintaining asks
Adding orders
Cancelling orders
Modifying orders
Matching orders
Calculating market metrics
MatchingEngine

Responsible for:

Receiving incoming orders
Calling the matching logic
Recording generated trades
Resting remaining limit orders
Complexity

For the current implementation:

Operation	Approximate Complexity
Add order	O(log P)
Best bid	O(1)
Best ask	O(1)
Spread	O(1)
Mid price	O(1)
Quantity at price	O(Q)
Cancel order	O(N)
Modify order	O(N)
Matching	O(K log P)

Where:

P = number of price levels
Q = number of orders at a price level
N = number of active orders
K = number of price levels/orders consumed during matching

The current implementation intentionally favors clarity and correctness. Further optimization is planned for high-throughput workloads.

Testing

The project includes unit tests covering core exchange behavior.

Current tests include:

Limit order insertion
Bid-side ordering
Ask-side ordering
Best bid
Best ask
Spread calculation
Mid-price calculation
Order cancellation
Order modification
Quantity tracking
Order matching
Partial fills
Price-time priority
Trade generation
Matching engine integration

Run the tests using:

cmake --build build
./build/test_order_book

Expected output:

OrderBook tests passed!
Build Instructions
Requirements
C++20 compatible compiler
CMake 3.20+
Git

Recommended development environments:

Linux
macOS
Unix-like systems
Clone Repository
git clone https://github.com/palrishabh06/market-microstructure-simulator.git
cd market-microstructure-simulator
Configure
cmake -S . -B build
Build
cmake --build build
Run Tests
./build/test_order_book
Example

A simple matching scenario:

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

The BUY order can execute against the SELL order because:

BUY price 106.00 >= SELL price 105.00

The resulting trade is:

Price:    105.00
Quantity: 40

The remaining SELL quantity becomes:

60
Design Principles

The project is designed around several important systems principles:

Deterministic Matching

Given the same sequence of orders, the matching engine should produce the same sequence of executions.

Separation of Responsibilities

The order book manages market state while the matching engine manages order submission and trade recording.

Explicit Data Structures

Standard library containers are used to represent price levels and FIFO order queues directly.

Testability

Core exchange functionality is separated into components that can be tested independently.

Extensibility

The architecture is designed to support future components such as:

Market-data replay
Strategy simulation
Backtesting
Performance benchmarking
Order-flow generation
Exchange latency simulation
Transaction costs
Slippage analysis
PnL calculation
Planned Improvements

The project is actively designed for further development.

Order Book Optimization
Faster order lookup
Order-ID indexing
Reduced cancellation complexity
More cache-friendly data structures
Integer price ticks instead of floating-point prices
Market Data

Planned support for:

Level-1 market data
Level-2 order book snapshots
Trade feeds
Order-flow statistics
Historical market-data replay
Exchange Simulation

Future components may include:

Exchange gateway simulation
Matching latency
Network latency
Order processing latency
Event queues
Deterministic event replay
Trading Research

Future research components:

Trading strategy simulation
Backtesting
PnL calculation
Transaction costs
Slippage
Market impact
Execution analysis
Performance Benchmarking

Planned benchmarks include:

Orders processed per second
Matching latency
Cancellation latency
Order insertion latency
Memory usage
Performance under deep order books
Performance under high order flow
Technology Stack
Core
C++20
STL
CMake
Data Structures
std::map
std::deque
std::vector
Development
Git
GDB
Linux/macOS
VS Code
Testing
C++ assertions
Custom unit tests
CMake build system
Why This Project?

Exchange matching engines are latency-sensitive systems where correctness, deterministic behavior, data structures, and performance are critical.

This project provides hands-on implementation of concepts that are fundamental to:

Algorithmic Trading
High-Frequency Trading
Market Microstructure
Exchange Infrastructure
Financial Systems
Low-Latency Systems
Quantitative Research
Performance Engineering

Rather than treating a trading system as a black box, this project implements the core matching process from the ground up.

Current Status

The core limit order book and matching engine are implemented and tested.

Current functionality includes:

✓ Limit orders
✓ Market order matching
✓ Price-time priority
✓ Partial fills
✓ Multi-level matching
✓ Trade generation
✓ Order cancellation
✓ Order modification
✓ Best bid / ask
✓ Spread
✓ Mid price
✓ Quantity-at-price
✓ Price-level tracking
✓ Matching engine
✓ Unit tests
✓ CMake build system

The project is being extended toward a more complete exchange and market-data simulation framework.

Disclaimer

This project is an educational and research-oriented exchange simulator.

It is not connected to any real financial exchange and should not be used for live trading or financial decision-making.

Author

Rishabh Raj

Computer Science & Engineering
Birla Institute of Technology, Mesra

GitHub: https://github.com/palrishabh06

LinkedIn: https://www.linkedin.com/in/palrishabh06