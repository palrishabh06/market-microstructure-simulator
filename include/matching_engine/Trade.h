#pragma once

#include <cstdint>

// Represents a single executed trade.
struct Trade {

    // ID of the order that initiated the trade.
    std::uint64_t incomingOrderId;

    // ID of the resting order that was already in the book.
    std::uint64_t restingOrderId;

    // Price at which the trade was executed.
    double price;

    // Quantity executed in this trade.
    std::uint64_t quantity;

    // Timestamp at which the trade occurred.
    std::uint64_t timestamp;
};