#pragma once
#include "broker-interface/order.hpp"
#include "data/bar.hpp"
#include "logging/logging.hpp"
#include <cstddef>
#include <list>
#include <memory>

namespace midas::backtest {

class SimulationOrderTransmitter : public midas::OrderVisitor {
  const Bar *bar;

public:
  SimulationOrderTransmitter(const Bar *bar) : bar(bar) {}

  virtual void visit(SimpleOrder &) override;
  virtual void visit(BracketedOrder &) override;
};

class BacktestOrderManager : public midas::OrderManager {
  std::list<std::shared_ptr<Order>> activeOrdersList, completedOrdersList;

public:
  BacktestOrderManager(std::shared_ptr<logging::thread_safe_logger_t> &logger)
      : midas::OrderManager(logger) {}
  virtual void transmit(std::shared_ptr<Order>) override;
  virtual bool hasActiveOrders() override;
  /**
   * Simulates what happens to the orders when the bar elapses.
   * i.e stop orders triggered, limit orders triggered, etc.
   */
  void simulate(const midas::Bar *);
  virtual std::list<Order *> getFilledOrders() override;
  std::size_t inline totalSize() {
    return activeOrdersList.size() + completedOrdersList.size();
  }
};

} // namespace midas::backtest