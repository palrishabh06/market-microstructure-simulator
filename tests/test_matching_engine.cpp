#include "matching_engine/MatchingEngine.h"

#include <cassert>
#include <iostream>

// Tests the basic functionality of the MatchingEngine.
int main() {

    // Create a fresh matching engine.
    MatchingEngine engine;

    // Create a resting SELL order.
    Order sellOrder{
        100,                // Order ID.
        Side::SELL,         // Order side.
        OrderType::LIMIT,   // Order type.
        101.00,             // Price.
        5,                  // Quantity.
        100                 // Timestamp.
    };

    // Submit the SELL order.
    // Since there is no matching BUY order,
    // it should rest in the order book.
    engine.submitOrder(sellOrder);

    // Verify that the SELL order is resting.
    assert(
        engine.getOrderBook().quantityAtPrice(
            Side::SELL,
            101.00
        ) == 5
    );

    // Create an incoming BUY order.
    Order buyOrder{
        101,                // Order ID.
        Side::BUY,          // Order side.
        OrderType::LIMIT,   // Order type.
        102.00,             // Maximum BUY price.
        5,                  // Quantity.
        101                 // Timestamp.
    };

    // Submit the BUY order.
    // It should match the resting SELL order.
    engine.submitOrder(buyOrder);

    // Exactly one trade should have been generated.
    assert(engine.getTrades().size() == 1);

    // Verify the incoming order ID.
    assert(engine.getTrades()[0].incomingOrderId == 101);

    // Verify the resting order ID.
    assert(engine.getTrades()[0].restingOrderId == 100);

    // Verify the execution price.
    assert(engine.getTrades()[0].price == 101.00);

    // Verify the executed quantity.
    assert(engine.getTrades()[0].quantity == 5);

    // The SELL order should now be completely filled.
    assert(
        engine.getOrderBook().quantityAtPrice(
            Side::SELL,
            101.00
        ) == 0
    );

    // Verify that clearing the trade history works.
    engine.clearTrades();

    // No trades should remain after clearing.
    assert(engine.getTrades().empty());

    // All MatchingEngine tests passed.
    std::cout << "MatchingEngine tests passed!" << std::endl;

    return 0;
}