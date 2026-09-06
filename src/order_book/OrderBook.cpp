#include "order_book/OrderBook.h"

#include <algorithm>

// Adds a limit order to the appropriate side of the order book.
void OrderBook::addLimitOrder(const Order& order) {

    // Make sure only LIMIT orders are added to the order book.
    if (order.type != OrderType::LIMIT) {
        return;
    }

    // Add BUY orders to the bid side.
    if (order.side == Side::BUY) {

        // The order is placed at its price level.
        // push_back preserves price-time priority.
        bids[order.price].push_back(order);
    }

    // Add SELL orders to the ask side.
    else {

        // The order is placed at its price level.
        // push_back preserves price-time priority.
        asks[order.price].push_back(order);
    }
}

// Cancels an active order using its unique order ID.
bool OrderBook::cancelOrder(std::uint64_t orderId) {

    // Search through all BUY price levels.
    for (auto bidLevel = bids.begin(); bidLevel != bids.end(); ++bidLevel) {

        // Search through every order at this price level.
        auto& orders = bidLevel->second;

        for (auto order = orders.begin(); order != orders.end(); ++order) {

            // Check whether this is the order we want to cancel.
            if (order->id == orderId) {

                // Remove the order from the price level.
                orders.erase(order);

                // If no orders remain at this price,
                // remove the entire price level.
                if (orders.empty()) {
                    bids.erase(bidLevel);
                }

                // Cancellation was successful.
                return true;
            }
        }
    }

    // Search through all SELL price levels.
    for (auto askLevel = asks.begin(); askLevel != asks.end(); ++askLevel) {

        // Search through every order at this price level.
        auto& orders = askLevel->second;

        for (auto order = orders.begin(); order != orders.end(); ++order) {

            // Check whether this is the order we want to cancel.
            if (order->id == orderId) {

                // Remove the order from the price level.
                orders.erase(order);

                // If no orders remain at this price,
                // remove the entire price level.
                if (orders.empty()) {
                    asks.erase(askLevel);
                }

                // Cancellation was successful.
                return true;
            }
        }
    }

    // The order ID was not found.
    return false;
}

// Modifies the remaining quantity of an active order.
bool OrderBook::modifyOrder(
    std::uint64_t orderId,
    std::uint64_t newQuantity
) {

    // Search through all BUY price levels.
    for (auto bidLevel = bids.begin(); bidLevel != bids.end(); ++bidLevel) {

        // Get all orders at this price level.
        auto& orders = bidLevel->second;

        // Search for the requested order.
        for (auto order = orders.begin(); order != orders.end(); ++order) {

            // Check whether this is the order we want to modify.
            if (order->id == orderId) {

                // A quantity of zero means the order should be removed.
                if (newQuantity == 0) {

                    // Remove the order from the price level.
                    orders.erase(order);

                    // Remove the price level if it is now empty.
                    if (orders.empty()) {
                        bids.erase(bidLevel);
                    }

                    // Modification was successful.
                    return true;
                }

                // Update the remaining quantity.
                order->quantity = newQuantity;

                // Modification was successful.
                return true;
            }
        }
    }

    // Search through all SELL price levels.
    for (auto askLevel = asks.begin(); askLevel != asks.end(); ++askLevel) {

        // Get all orders at this price level.
        auto& orders = askLevel->second;

        // Search for the requested order.
        for (auto order = orders.begin(); order != orders.end(); ++order) {

            // Check whether this is the order we want to modify.
            if (order->id == orderId) {

                // A quantity of zero means the order should be removed.
                if (newQuantity == 0) {

                    // Remove the order from the price level.
                    orders.erase(order);

                    // Remove the price level if it is now empty.
                    if (orders.empty()) {
                        asks.erase(askLevel);
                    }

                    // Modification was successful.
                    return true;
                }

                // Update the remaining quantity.
                order->quantity = newQuantity;

                // Modification was successful.
                return true;
            }
        }
    }

    // The order ID was not found.
    return false;
}

// Matches an incoming order against the opposite side of the book.
std::vector<Trade> OrderBook::matchOrder(Order& incomingOrder) {

    // Store all trades generated by this matching operation.
    std::vector<Trade> generatedTrades;

    // Handle an incoming BUY order.
    if (incomingOrder.side == Side::BUY) {

        // Continue while the BUY order still has quantity
        // and there are SELL orders available.
        while (incomingOrder.quantity > 0 && !asks.empty()) {

            // The lowest ask is the best available SELL price.
            auto askLevel = asks.begin();

            // Read the best ask price.
            double askPrice = askLevel->first;

            // A LIMIT BUY can only execute when its price is
            // greater than or equal to the best ask.
            if (incomingOrder.type == OrderType::LIMIT &&
                incomingOrder.price < askPrice) {
                break;
            }

            // Get the orders waiting at the best ask price.
            auto& restingOrders = askLevel->second;

            // The first order has time priority.
            Order& restingOrder = restingOrders.front();

            // Determine how much quantity can actually trade.
            std::uint64_t executedQuantity =
                std::min(incomingOrder.quantity, restingOrder.quantity);

            // Create a record of the executed trade.
            Trade trade{
                incomingOrder.id,       // Incoming order ID.
                restingOrder.id,        // Resting order ID.
                restingOrder.price,     // Execution price.
                executedQuantity,       // Executed quantity.
                incomingOrder.timestamp  // Trade timestamp.
            };

            // Store the generated trade.
            generatedTrades.push_back(trade);

            // Reduce the quantity of the incoming order.
            incomingOrder.quantity -= executedQuantity;

            // Reduce the quantity of the resting order.
            restingOrder.quantity -= executedQuantity;

            // If the resting order has been completely filled,
            // remove it from the front of the queue.
            if (restingOrder.quantity == 0) {
                restingOrders.pop_front();
            }

            // If no orders remain at this price level,
            // remove the price level from the book.
            if (restingOrders.empty()) {
                asks.erase(askLevel);
            }
        }
    }

    // Handle an incoming SELL order.
    else {

        // Continue while the SELL order still has quantity
        // and there are BUY orders available.
        while (incomingOrder.quantity > 0 && !bids.empty()) {

            // The highest bid is the best available BUY price.
            auto bidLevel = bids.begin();

            // Read the best bid price.
            double bidPrice = bidLevel->first;

            // A LIMIT SELL can only execute when its price is
            // less than or equal to the best bid.
            if (incomingOrder.type == OrderType::LIMIT &&
                incomingOrder.price > bidPrice) {
                break;
            }

            // Get the orders waiting at the best bid price.
            auto& restingOrders = bidLevel->second;

            // The first order has time priority.
            Order& restingOrder = restingOrders.front();

            // Determine how much quantity can actually trade.
            std::uint64_t executedQuantity =
                std::min(incomingOrder.quantity, restingOrder.quantity);

            // Create a record of the executed trade.
            Trade trade{
                incomingOrder.id,       // Incoming order ID.
                restingOrder.id,        // Resting order ID.
                restingOrder.price,     // Execution price.
                executedQuantity,       // Executed quantity.
                incomingOrder.timestamp  // Trade timestamp.
            };

            // Store the generated trade.
            generatedTrades.push_back(trade);

            // Reduce the quantity of the incoming order.
            incomingOrder.quantity -= executedQuantity;

            // Reduce the quantity of the resting order.
            restingOrder.quantity -= executedQuantity;

            // If the resting order has been completely filled,
            // remove it from the front of the queue.
            if (restingOrder.quantity == 0) {
                restingOrders.pop_front();
            }

            // If no orders remain at this price level,
            // remove the price level from the book.
            if (restingOrders.empty()) {
                bids.erase(bidLevel);
            }
        }
    }

    // Return every trade generated by this matching operation.
    return generatedTrades;
}

// Returns the highest bid price.
double OrderBook::bestBid() const {

    // If there are no buy orders, return 0.
    if (bids.empty()) {
        return 0.0;
    }

    // Because bids are sorted from highest to lowest,
    // the first element contains the highest bid.
    return bids.begin()->first;
}

// Returns the lowest ask price.
double OrderBook::bestAsk() const {

    // If there are no sell orders, return 0.
    if (asks.empty()) {
        return 0.0;
    }

    // Because asks are sorted from lowest to highest,
    // the first element contains the lowest ask.
    return asks.begin()->first;
}

// Returns the bid-ask spread.
double OrderBook::spread() const {

    // A spread cannot be calculated unless both sides exist.
    if (bids.empty() || asks.empty()) {
        return 0.0;
    }

    // Spread = lowest ask - highest bid.
    return bestAsk() - bestBid();
}

// Returns the midpoint between the best bid and best ask.
double OrderBook::midPrice() const {

    // A midpoint cannot be calculated unless both sides exist.
    if (bids.empty() || asks.empty()) {
        return 0.0;
    }

    // Calculate the midpoint of the best prices.
    return (bestBid() + bestAsk()) / 2.0;
}

// Returns the total quantity available at a specific price.
std::uint64_t OrderBook::quantityAtPrice(
    Side side,
    double price
) const {

    // Start with zero quantity at this price.
    std::uint64_t totalQuantity = 0;

    // Check the bid side.
    if (side == Side::BUY) {

        // Find the requested price level.
        auto it = bids.find(price);

        // If the price level does not exist, return zero.
        if (it == bids.end()) {
            return 0;
        }

        // Add the remaining quantity of every order
        // at this price level.
        for (const Order& order : it->second) {
            totalQuantity += order.quantity;
        }
    }

    // Check the ask side.
    else {

        // Find the requested price level.
        auto it = asks.find(price);

        // If the price level does not exist, return zero.
        if (it == asks.end()) {
            return 0;
        }

        // Add the remaining quantity of every order
        // at this price level.
        for (const Order& order : it->second) {
            totalQuantity += order.quantity;
        }
    }

    // Return the total quantity at the requested price.
    return totalQuantity;
}

// Returns the number of active price levels.
std::size_t OrderBook::priceLevelCount(Side side) const {

    // Return the number of bid price levels.
    if (side == Side::BUY) {
        return bids.size();
    }

    // Return the number of ask price levels.
    return asks.size();
}