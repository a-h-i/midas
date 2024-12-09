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
  auto &createdOrder =
      currentOrders.emplace_back(std::make_shared<BracketedOrder>(
          quantity, direction, instrument, entryPrice, profitPrice,
          stopLossPrice, logger));
  createdOrder->addStatusChangeListener(
      std::bind(&Trader::handleOrderStatusChangeEvent, this,
                std::placeholders::_1, std::placeholders::_2));
  orderManager->transmit(createdOrder);
  auto updatedSummary = summary.summary();
  updatedSummary.hasOpenPosition = this->hasOpenPosition();
  summarySignal(updatedSummary);
}

bool midas::trader::Trader::hasOpenPosition() { return !currentOrders.empty(); }

boost::signals2::connection midas::trader::Trader::connectSlot(
    const trade_summary_signal_t::slot_type &subscriber) {
  return summarySignal.connect(subscriber);
}

void midas::trader::Trader::handleOrderStatusChangeEvent(
    Order &order, Order::StatusChangeEvent event) {
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
      summary.addToSummary(itr->get());
      // We then remove the order from the current orders list
      currentOrders.erase(itr);
      // Finally we wish to invoke the signal handler but not while holding the
      // scoped lock.
      updatedSummary = summary.summary();
      updatedSummary->hasOpenPosition = this->hasOpenPosition();
    }
  }
  if (updatedSummary.has_value()) {
    summarySignal(updatedSummary.value());
  }
}
