#include "order_book/OrderBook.h"
#include "matching_engine/MatchingEngine.h"
#include "matching_engine/Trade.h"

#include <cassert>
#include <iostream>
#include <vector>

// Tests the basic functionality of the OrderBook.
int main() {

    // ------------------------------------------------------------
    // TEST 1: Basic order book functionality
    // ------------------------------------------------------------

    // Create an empty order book.
    OrderBook book;

    // Create a BUY limit order at 100.00.
    Order buyOrder{
        1,                  // Order ID.
        Side::BUY,          // Order side.
        OrderType::LIMIT,   // Order type.
        100.00,             // Price.
        10,                 // Quantity.
        1                   // Timestamp.
    };

    // Create a SELL limit order at 101.00.
    Order sellOrder{
        2,                  // Order ID.
        Side::SELL,         // Order side.
        OrderType::LIMIT,   // Order type.
        101.00,             // Price.
        5,                  // Quantity.
        2                   // Timestamp.
    };

    // Add both orders to the order book.
    book.addLimitOrder(buyOrder);
    book.addLimitOrder(sellOrder);

    // Verify the highest bid.
    assert(book.bestBid() == 100.00);

    // Verify the lowest ask.
    assert(book.bestAsk() == 101.00);

    // Verify the bid-ask spread.
    assert(book.spread() == 1.00);

    // Verify the midpoint.
    assert(book.midPrice() == 100.50);

    // Verify quantity at the bid price.
    assert(book.quantityAtPrice(Side::BUY, 100.00) == 10);

    // Verify quantity at the ask price.
    assert(book.quantityAtPrice(Side::SELL, 101.00) == 5);

    // Verify the number of price levels on each side.
    assert(book.priceLevelCount(Side::BUY) == 1);
    assert(book.priceLevelCount(Side::SELL) == 1);


    // ------------------------------------------------------------
    // TEST 2: Order cancellation
    // ------------------------------------------------------------

    // Cancel the BUY order using its unique ID.
    bool cancelled = book.cancelOrder(1);

    // Verify that the cancellation was successful.
    assert(cancelled == true);

    // Verify that the BUY price level was removed.
    assert(book.priceLevelCount(Side::BUY) == 0);

    // Verify that there is no quantity remaining at 100.00.
    assert(book.quantityAtPrice(Side::BUY, 100.00) == 0);

    // Try cancelling the same order again.
    bool cancelledAgain = book.cancelOrder(1);

    // The second cancellation should fail because
    // the order has already been removed.
    assert(cancelledAgain == false);


    // ------------------------------------------------------------
    // TEST 3: Order modification
    // ------------------------------------------------------------

    // Create a new BUY order for modification testing.
    Order modifyTestOrder{
        3,                  // Order ID.
        Side::BUY,          // Order side.
        OrderType::LIMIT,   // Order type.
        99.00,              // Price.
        20,                 // Initial quantity.
        3                   // Timestamp.
    };

    // Add the new order to the order book.
    book.addLimitOrder(modifyTestOrder);

    // Verify that the order was added with quantity 20.
    assert(book.quantityAtPrice(Side::BUY, 99.00) == 20);

    // Change the quantity from 20 to 12.
    bool modified = book.modifyOrder(3, 12);

    // Verify that the modification succeeded.
    assert(modified == true);

    // Verify that the quantity is now 12.
    assert(book.quantityAtPrice(Side::BUY, 99.00) == 12);

    // Modify the same order to quantity zero.
    bool removedByModification = book.modifyOrder(3, 0);

    // Verify that the modification succeeded.
    assert(removedByModification == true);

    // A zero quantity should remove the order completely.
    assert(book.quantityAtPrice(Side::BUY, 99.00) == 0);

    // Verify that the empty price level was also removed.
    assert(book.priceLevelCount(Side::BUY) == 0);

    // Try modifying an order that no longer exists.
    bool modifiedAgain = book.modifyOrder(3, 5);

    // This should fail because order 3 was removed.
    assert(modifiedAgain == false);


    // ------------------------------------------------------------
    // TEST 4: BUY order matching against SELL order
    // ------------------------------------------------------------

    // Create a separate order book for matching tests.
    // This prevents earlier tests from affecting the matching state.
    OrderBook matchingBook;

    // Create a resting SELL order.
    Order restingSell{
        10,                 // Order ID.
        Side::SELL,         // Order side.
        OrderType::LIMIT,   // Order type.
        101.00,             // Price.
        5,                  // Quantity.
        10                  // Timestamp.
    };

    // Add the SELL order to the matching book.
    matchingBook.addLimitOrder(restingSell);

    // Create an incoming BUY order.
    // It wants 7 units at a maximum price of 102.00.
    Order incomingBuy{
        11,                 // Order ID.
        Side::BUY,          // Order side.
        OrderType::LIMIT,   // Order type.
        102.00,             // Price.
        7,                  // Quantity.
        11                  // Timestamp.
    };

    // Match the incoming BUY against the resting SELL.
    std::vector<Trade> trades =
        matchingBook.matchOrder(incomingBuy);

    // Exactly one trade should have occurred.
    assert(trades.size() == 1);

    // Verify the incoming order ID.
    assert(trades[0].incomingOrderId == 11);

    // Verify the resting order ID.
    assert(trades[0].restingOrderId == 10);

    // The trade executes at the resting SELL price.
    assert(trades[0].price == 101.00);

    // Only 5 units were available, so only 5 should execute.
    assert(trades[0].quantity == 5);

    // The incoming BUY order should have 2 units remaining.
    assert(incomingBuy.quantity == 2);

    // The original SELL order should be completely filled.
    assert(
        matchingBook.quantityAtPrice(Side::SELL, 101.00) == 0
    );


    // ------------------------------------------------------------
    // TEST 5: Price-time priority
    // ------------------------------------------------------------

    // Create a separate order book for priority testing.
    OrderBook priorityBook;

    // First SELL order at 101.00.
    Order firstSell{
        20,                 // Order ID.
        Side::SELL,         // Order side.
        OrderType::LIMIT,   // Order type.
        101.00,             // Price.
        5,                  // Quantity.
        20                  // Earlier timestamp.
    };

    // Second SELL order at the same price.
    Order secondSell{
        21,                 // Order ID.
        Side::SELL,         // Order side.
        OrderType::LIMIT,   // Order type.
        101.00,             // Price.
        7,                  // Quantity.
        21                  // Later timestamp.
    };

    // Add the first order before the second order.
    priorityBook.addLimitOrder(firstSell);
    priorityBook.addLimitOrder(secondSell);

    // Create a BUY order large enough to partially consume both sells.
    Order priorityBuy{
        30,                 // Order ID.
        Side::BUY,          // Order side.
        OrderType::LIMIT,   // Order type.
        101.00,             // Price.
        8,                  // Quantity.
        30                  // Timestamp.
    };

    // Match the BUY order against the two SELL orders.
    std::vector<Trade> priorityTrades =
        priorityBook.matchOrder(priorityBuy);

    // Two trades should have been generated.
    assert(priorityTrades.size() == 2);

    // The first SELL order must execute first.
    assert(priorityTrades[0].restingOrderId == 20);

    // The first order had 5 units available.
    assert(priorityTrades[0].quantity == 5);

    // The second SELL order should execute second.
    assert(priorityTrades[1].restingOrderId == 21);

    // Only 3 units of the second order should execute.
    assert(priorityTrades[1].quantity == 3);

    // The BUY order should now be completely filled.
    assert(priorityBuy.quantity == 0);

    // The second SELL order should have 4 units remaining.
    assert(
        priorityBook.quantityAtPrice(Side::SELL, 101.00) == 4
    );

    // ------------------------------------------------------------
    // TEST 6: MatchingEngine integration
    // ------------------------------------------------------------

    // Create a fresh matching engine.
    MatchingEngine engine;

    // Create a SELL order that will rest in the book.
    Order engineSell{
        40,                 // Order ID.
        Side::SELL,         // Order side.
        OrderType::LIMIT,   // Order type.
        105.00,             // Price.
        5,                  // Quantity.
        40                  // Timestamp.
    };

    // Submit the SELL order to the matching engine.
    engine.submitOrder(engineSell);

    // Verify that the SELL order is now resting
    // in the order book.
    assert(
        engine.getOrderBook().quantityAtPrice(
            Side::SELL,
            105.00
        ) == 5
    );

    // Create an incoming BUY order.
    // It wants 8 units but only 5 are available.
    Order engineBuy{
        41,                 // Order ID.
        Side::BUY,          // Order side.
        OrderType::LIMIT,   // Order type.
        106.00,             // Price.
        8,                  // Quantity.
        41                  // Timestamp.
    };

    // Submit the BUY order to the matching engine.
    engine.submitOrder(engineBuy);

    // Exactly one trade should have occurred.
    assert(engine.getTrades().size() == 1);

    // Verify that 5 units were executed.
    assert(engine.getTrades()[0].quantity == 5);

    // Verify the execution price.
    assert(engine.getTrades()[0].price == 105.00);

    // The original SELL order should now be completely filled.
    assert(
        engine.getOrderBook().quantityAtPrice(
            Side::SELL,
            105.00
        ) == 0
    );

    // The remaining 3 BUY units should now rest
    // in the order book at 106.00.
    assert(
        engine.getOrderBook().quantityAtPrice(
            Side::BUY,
            106.00
        ) == 3
    );

    // Verify that the BUY order is now the best bid.
    assert(engine.getOrderBook().bestBid() == 106.00);

    // ------------------------------------------------------------
    // ALL TESTS PASSED
    // ------------------------------------------------------------

    // Print a success message if every assertion passed.
    std::cout << "OrderBook tests passed!" << std::endl;

    return 0;
}