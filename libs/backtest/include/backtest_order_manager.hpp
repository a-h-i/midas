#pragma once
#include "broker-interface/order.hpp"
#include "data/bar.hpp"
#include "logging/logging.hpp"
#include <list>
#include <memory>
namespace midas::backtest {

class BacktestOrderManager : public midas::OrderManager {
  std::list<std::shared_ptr<Order>> activeOrdersList, completedOrdersList;

public:
  BacktestOrderManager(std::shared_ptr<logging::thread_safe_logger_t> logger);
  virtual void transmit(std::shared_ptr<Order>) override;
  virtual bool hasActiveOrders() const override;
  /**
   * Simulates what happens to the orders when the bar elapses.
   * i.e stop orders triggered, limit orders triggered, etc.
   */
  void simulate(const midas::Bar &);
};

} // namespace midas::backtest