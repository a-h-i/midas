#pragma once
#include "CommonDefs.h"
#include "Contract.h"
#include "Order.h"
#include "broker-interface/instruments.hpp"
#include "broker-interface/order.hpp"
#include "ibkr-driver/ibkr.hpp"
#include <atomic>
#include <memory>

namespace ibkr::internal {

struct OrderInfo {
  midas::InstrumentEnum instrument;
  Contract ibkrContract;
  Order ibkrOrder;
};

class OrderManager : public midas::OrderManager {
  Driver &driver;

public:
  OrderManager(std::shared_ptr<logging::thread_safe_logger_t> logger, Driver &);
  virtual void transmit(std::shared_ptr<midas::Order>) override;
  virtual bool hasActiveOrders() const override;
  virtual std::generator<midas::Order *> getFilledOrders() override;
};

std::list<OrderInfo> transformOrder(midas::Order &,
                                    std::atomic<OrderId> &orderCtr);

}; // namespace ibkr::internal