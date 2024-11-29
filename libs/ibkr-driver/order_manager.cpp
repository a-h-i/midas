#include "broker-interface/order.hpp"
#include "ibkr/internal/client.hpp"
#include "ibkr/internal/ibkr_order_manager.hpp"
#include <memory>
ibkr::internal::OrderManager::OrderManager(
    std::shared_ptr<logging::thread_safe_logger_t> logger, Driver &driver)
    : midas::OrderManager(logger), driver(driver) {}

void ibkr::internal::OrderManager::transmit(
    std::shared_ptr<midas::Order> orderPtr) {
  auto transformedOrders = transformOrder(*orderPtr, driver.orderCtr());
  transmittedOrders.push_back(orderPtr);
  // we need to listen for order fill status and cancel events
  orderPtr->addStatusChangeListener(
      midas::Order::status_change_signal_t::slot_type(
          [this, &orderPtr](midas::Order &order,
                            midas::Order::StatusChangeEvent event) {
            auto removePredicate = [&order](const auto &elemPtr) {
              return (*elemPtr) == order;
            };

            if (event.newStatus == midas::OrderStatusEnum::Cancelled) {
              transmittedOrders.remove_if(removePredicate);
              cancelledOrders.push_back(orderPtr);
            } else if (event.newStatus == midas::OrderStatusEnum::Filled) {
              transmittedOrders.remove_if(removePredicate);
              filledOrders.push_back(orderPtr);
            }
          })
          .track_foreign(orderPtr));
  // queue for transmission
  driver.implementation->transmitOrder(transformedOrders);
}
bool ibkr::internal::OrderManager::hasActiveOrders() const {
  return !transmittedOrders.empty();
}
std::generator<midas::Order *> ibkr::internal::OrderManager::getFilledOrders() {

  for (auto &order : filledOrders) {
    co_yield order.get();
  }
}