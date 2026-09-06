#pragma once

#include <cstdint>

// Represents the side of an order.
enum class Side {
    BUY,
    SELL
};

// Represents the type of an order.
enum class OrderType {
    LIMIT,
    MARKET
};

// Represents a single order submitted to the exchange.
struct Order {

    // Unique identifier for the order.
    std::uint64_t id;

    // BUY or SELL.
    Side side;

    // LIMIT or MARKET.
    OrderType type;

    // Price at which the trader wants to trade.
    // Market orders will not use this value.
    double price;

    // Number of units remaining to be executed.
    std::uint64_t quantity;

    // Arrival timestamp used for price-time priority.
    std::uint64_t timestamp;
};
