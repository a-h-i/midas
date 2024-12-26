#include "trader/base_trader.hpp"
#include "broker-interface/order.hpp"
#include "broker-interface/trades_summary.hpp"
#include "logging/logging.hpp"
#include <algorithm>
#include <functional>
#include <mutex>
#include <optional>
void midas::trader::Trader::enterBracket(
    InstrumentEnum instrument, unsigned int quantity, OrderDirection direction,
    double entryPrice, double stopLossPrice, double profitPrice) {
  if (paused()) {
    return;
  }
  std::scoped_lock enterBracketLock(orderStateMutex);
  INFO_LOG(*logger) << "Entering order for " << instrument << " x" << quantity
                    << " " << direction << "" << " entryPrice: " << entryPrice
                    << " stopLossLimit: " << stopLossPrice
                    << " profitLimit: " << profitPrice;
  auto createdOrder = std::make_shared<BracketedOrder>(
      quantity, direction, instrument, entryPrice, profitPrice, stopLossPrice,
      logger);
  enterBracket(createdOrder);
}

void midas::trader::Trader::enterBracket(
    std::shared_ptr<BracketedOrder> order) {
  std::scoped_lock enterBracketLock(orderStateMutex);
  order->addStatusChangeListener(
      std::bind(&Trader::handleOrderStatusChangeEvent, this,
                std::placeholders::_1, std::placeholders::_2));
  currentOrders.push_back(order);
  orderManager->transmit(order);
  auto updatedSummary = summary.summary();
  updatedSummary.hasOpenPosition = this->hasOpenPosition();
  summarySignal(updatedSummary);
}
bool midas::trader::Trader::hasOpenPosition() {
  std::scoped_lock enterBracketLock(orderStateMutex);
  return !currentOrders.empty();
}

boost::signals2::connection midas::trader::Trader::connectSummarySlot(
    const trade_summary_signal_t::slot_type &subscriber) {
  return summarySignal.connect(subscriber);
}

boost::signals2::connection midas::trader::Trader::connectDecisionParamsSlot(
    const decision_params_signal_t::slot_type &subscriber) {
  return decisionParamsSignal.connect(subscriber);
}

void midas::trader::Trader::handleOrderStatusChangeEvent(
    Order &order, Order::StatusChangeEvent event) {
  DEBUG_LOG(*logger) << "Base trader handling order status change new state: "
                     << event.newStatus;
  std::optional<TradeSummary> updatedSummary;
  if (event.newStatus == OrderStatusEnum::Filled ||
      event.newStatus == OrderStatusEnum::Cancelled) {
    std::scoped_lock orderStateChangeLock(orderStateMutex);
    auto itr =
        std::find_if(currentOrders.begin(), currentOrders.end(),
                     [&order](auto current) { return *current == order; });
    if (itr != currentOrders.end()) {
      executedOrders.push_back(*itr);
      // we need to update summary

      if (event.newStatus == OrderStatusEnum::Filled) {
        DEBUG_LOG(*logger) << "updating filled summary";
        summary.addToSummary(itr->get());
        updatedSummary = summary.summary();
      }
      // We then remove the order from the current orders list
      currentOrders.erase(itr);
      // Finally we wish to invoke the signal handler but not while holding the
      // scoped lock.
      ;
    } else {
      ERROR_LOG(*logger) << "Base trader could not find order";
    }
  }
  {
    DEBUG_LOG(*logger) << "None accepted or filled state " << event.newStatus;
  }
  if (updatedSummary.has_value()) {
    updatedSummary->hasOpenPosition = this->hasOpenPosition();
    summarySignal(updatedSummary.value());
  }
}
