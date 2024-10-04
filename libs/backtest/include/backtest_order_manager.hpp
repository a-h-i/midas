#pragma once
#include "broker-interface/order.hpp"
#include "data/bar.hpp"
#include "logging/logging.hpp"
#include <functional>
#include <list>
#include <memory>
namespace midas::backtest {

class SimulationOrderTransmitter : public midas::OrderTransmitter {
  const Bar &bar;
  std::function<void(double fillPrice, double commission, bool isFinished)> triggerCallback;

public:
  SimulationOrderTransmitter(const Bar &bar, std::function<void(double fillPrice, double commission, bool isFinished)> triggerCallback)
      : bar(bar), triggerCallback(triggerCallback) {}
  virtual void transmit(SimpleOrder &) override;
  virtual void transmit(BracketedOrder &) override;
};

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