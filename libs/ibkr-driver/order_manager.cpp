#include "broker-interface/instruments.hpp"
#include "broker-interface/order.hpp"
#include "ibkr/internal/client.hpp"
#include "ibkr/internal/ibkr_order_manager.hpp"
#include "logging/logging.hpp"
#include <memory>
#include <mutex>
ibkr::internal::OrderManager::OrderManager(
    Driver &driver, std::shared_ptr<logging::thread_safe_logger_t> &logger)
    : midas::OrderManager(logger), driver(driver) {}

void ibkr::internal::OrderManager::transmit(
    std::shared_ptr<midas::Order> orderPtr) {
  std::scoped_lock ordersLock(ordersMutex);
  auto transformedOrders = transformOrder(*orderPtr, driver.orderCtr(), logger);
  transmittedOrders.push_back(orderPtr);
  // we need to listen for order fill status and cancel events
  orderPtr->addStatusChangeListener(
      midas::Order::status_change_signal_t::slot_type(
          [this, orderPtr](midas::Order &order,
                           midas::Order::StatusChangeEvent event) {
            std::scoped_lock ordersLock(ordersMutex);
            INFO_LOG(*logger) << "IBKR order manager status change listener "
                                 "triggered for internal id: "
                              << order.id;
            auto removePredicate = [&order](const auto &elemPtr) {
              return (*elemPtr) == order;
            };

            if (event.newStatus == midas::OrderStatusEnum::Cancelled) {
              transmittedOrders.remove_if(removePredicate);
              cancelledOrders.push_back(orderPtr);
            } else if (event.newStatus == midas::OrderStatusEnum::Filled) {
              transmittedOrders.remove_if(removePredicate);
              filledOrders.push_back(orderPtr);
              handlePnLUpdate(order.instrument, order.direction,
                              order.getAvgFillPrice() *
                                  order.getFilledQuantity());
            }
          })
          .track_foreign(orderPtr));
  // queue for transmission
  driver.implementation->transmitOrder(transformedOrders);
}
bool ibkr::internal::OrderManager::hasActiveOrders() {
  std::scoped_lock ordersLock(ordersMutex);
  return !transmittedOrders.empty();
}
std::list<midas::Order *> ibkr::internal::OrderManager::getFilledOrders() {
  std::scoped_lock ordersLock(ordersMutex);
  std::list<midas::Order *> orderPtrs;

  for (auto &orderPtr : filledOrders) {
    orderPtrs.push_back(orderPtr.get());
  }
  return orderPtrs;
}

void ibkr::internal::OrderManager::handlePnLUpdate(
    midas::InstrumentEnum instrument [[maybe_unused]],
    midas::OrderDirection direction, double price) {
  INFO_LOG(*logger) << "IBKR order manager handling pnl update";
  double directionModifier = direction == midas::OrderDirection::BUY ? 1 : -1;
  double adjustedPrice = price * directionModifier;

  const double realized = realizedPnl.fetch_add(adjustedPrice);
  realizedPnlSignal(realized);
}