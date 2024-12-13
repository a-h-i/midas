#pragma once
#include "CommonDefs.h"
#include "broker-interface/instruments.hpp"
#include "broker-interface/order.hpp"
#include "ibkr-driver/ibkr.hpp"
#include "ibkr/internal/order_wrapper.hpp"
#include "logging/logging.hpp"
#include <atomic>
#include <boost/unordered/concurrent_flat_map.hpp>
#include <list>
#include <memory>
#include <mutex>
namespace ibkr::internal {

class OrderManager : public midas::OrderManager {
  Driver &driver;
  std::list<std::shared_ptr<midas::Order>> transmittedOrders, filledOrders,
      cancelledOrders;
  std::recursive_mutex pnlMutex, ordersMutex;
  std::unordered_map<midas::InstrumentEnum, double> unrealizedPnl;
  std::atomic<double> realizedPnl{0};

  void handlePnLUpdate(midas::InstrumentEnum, midas::OrderDirection direction,
                       double price);

public:
  OrderManager(Driver &, std::shared_ptr<logging::thread_safe_logger_t> &logger);
  virtual void transmit(std::shared_ptr<midas::Order>) override;
  virtual bool hasActiveOrders() override;
  virtual std::list<midas::Order *> getFilledOrders() override;
};

/**
 Transforms a midas order into one or more IBKR orders.
 \note Orders should be transmitted in returned order.
 \param orderCtr order id counter
 */
std::list<std::shared_ptr<NativeOrder>>
transformOrder(midas::Order &, std::atomic<OrderId> &orderCtr, std::shared_ptr<logging::thread_safe_logger_t>);

}; // namespace ibkr::internal