#include "broker-interface/order.hpp"
#include "trader/trader.hpp"
#include <memory>

void midas::trader::Trader::enterBracket(
    InstrumentEnum instrument, unsigned int quantity, OrderDirection direction,
    double entryPrice, double stopLossPrice, double profitPrice) {
  auto& order =currentOrders.emplace_back(std::make_shared<BracketedOrder>(
      quantity, direction, instrument, entryPrice, profitPrice, stopLossPrice, logger));
  orderManager->transmit(order);
}