#pragma once
#include "CommonDefs.h"
#include "broker-interface/order.hpp"
#include "ibkr-driver/ibkr.hpp"
#include "ibkr/internal/order_wrapper.hpp"
#include <atomic>
#include <list>
#include <memory>
namespace ibkr::internal {

class OrderManager : public midas::OrderManager {
  Driver &driver;
  std::list<std::shared_ptr<midas::Order>> transmittedOrders, acceptedOrders,
      filledOrders, cancelledOrders;

public:
  OrderManager(std::shared_ptr<logging::thread_safe_logger_t> logger, Driver &);
  virtual void transmit(std::shared_ptr<midas::Order>) override;
  virtual bool hasActiveOrders() const override;
  virtual std::generator<midas::Order *> getFilledOrders() override;
};

/**
 Transforms a midas order into one or more IBKR orders.
 \note Orders should be transmitted in returned order.
 \param orderCtr order id counter
 */
std::list<std::shared_ptr<NativeOrder>>
transformOrder(midas::Order &, std::atomic<OrderId> &orderCtr);

}; // namespace ibkr::internal