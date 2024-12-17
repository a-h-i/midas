#include "broker-interface/order.hpp"
#include <algorithm>
#include <broker-interface/order_summary.hpp>
#include <stdexcept>

using namespace midas;

void OrderSummaryTracker::addToSummary(Order *order) {
  if (order->state() == OrderStatusEnum::Filled) {
    order->visit(*this);
  } else {
    throw std::logic_error("Attempted to add unfilled order to summary");
  }
}
void OrderSummaryTracker::visit(SimpleOrder &) {}

double getFillPriceWithSign(Order &order) {
  return order.direction == OrderDirection::SELL
             ? -order.getAvgFillPrice() * order.getFilledQuantity()
             : order.getAvgFillPrice() * order.getFilledQuantity();
}

void OrderSummaryTracker::visit(BracketedOrder &order) {
  accumulator.numberOfEntryOrdersTriggered++;
  auto &entryOrder = order.getEntryOrder();
  auto &stopOrder = order.getStopOrder();
  auto &profitOrder = order.getProfitTakerOrder();
  double entryPrice = getFillPriceWithSign(entryOrder);
  double orderBalance = 0;
  if (stopOrder.state() == OrderStatusEnum::Filled) {
    accumulator.numberOfStopLossTriggered++;
    double stopBalance = getFillPriceWithSign(stopOrder);
    double diff = entryPrice + stopBalance;
    orderBalance += diff * 2;
    accumulator.maxDownTurn = std::max(accumulator.maxDownTurn, std::abs(diff * 2));
  } else if (profitOrder.state() == OrderStatusEnum::Filled) {
    accumulator.numberOfProfitTakersTriggered++;
    double profitBalance = getFillPriceWithSign(profitOrder);
    double diff = entryPrice + profitBalance;
    orderBalance += diff * 2;
    accumulator.maxUpTurn = std::max(accumulator.maxUpTurn, std::abs(diff * 2));
  } else {
    throw std::logic_error("Attempted to get summary for bracket order that "
                           "has neither stop or profit taker filled");
  }
  accumulator.endingBalance += orderBalance;
  accumulator.successRatio =
      static_cast<double>(accumulator.numberOfProfitTakersTriggered) /
      accumulator.numberOfEntryOrdersTriggered;
}