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

void OrderSummaryTracker::visit(BracketedOrder &order) {
  accumulator.numberOfEntryOrdersTriggered++;
  auto &entryOrder = order.getEntryOrder();
  auto &stopOrder = order.getStopOrder();
  auto &profitOrder = order.getProfitTakerOrder();
  double orderBalance =
      entryOrder.getAvgFillPrice() * entryOrder.getFilledQuantity();
  if (entryOrder.direction == OrderDirection::BUY) {
    orderBalance *= -1;
  }

  if (stopOrder.state() == OrderStatusEnum::Filled) {
    accumulator.numberOfStopLossTriggered++;
    double stopBalance =
        stopOrder.getAvgFillPrice() * stopOrder.getFilledQuantity();
    accumulator.maxDownTurn = std::max(accumulator.maxDownTurn, stopBalance);
    if (stopOrder.direction == OrderDirection::BUY) {
      orderBalance -= stopBalance;
    } else {
      orderBalance += stopBalance;
    }
  } else if (profitOrder.state() == OrderStatusEnum::Filled) {
    accumulator.numberOfProfitTakersTriggered++;
    double profitBalance =
        profitOrder.getAvgFillPrice() * profitOrder.getFilledQuantity();

    accumulator.maxUpTurn = std::max(accumulator.maxUpTurn, profitBalance);
    if (profitOrder.direction == OrderDirection::BUY) {
      orderBalance -= profitBalance;
    } else {
      orderBalance += profitBalance;
    }
  } else {
    throw std::logic_error("Attempted to get summary for bracket order that "
                           "has neither stop or profit taker filled");
  }
  accumulator.endingBalance += orderBalance;
  accumulator.successRatio =
      static_cast<double>(accumulator.numberOfProfitTakersTriggered) /
      accumulator.numberOfEntryOrdersTriggered;
}