#pragma once

#include "broker-interface/order.hpp"
#include "broker-interface/subscription.hpp"
#include <memory>
namespace midas {
class Broker {

public:
  virtual ~Broker() = default;

  /**
   * Does nothing if not connected
   */
  virtual void connect() = 0;
  /**
   * Does nothing if connected
   */
  virtual void disconnect() = 0;
  virtual bool isConnected() const = 0;

  /**
   * Registers a data subscription
   */
  virtual void
  addSubscription(std::weak_ptr<midas::Subscription> subscription) = 0;

  /**
   * Processes pending events and actions.
   * Usually called from a dedicated thread.
   */
  virtual bool processCycle() = 0;

  virtual unsigned int estimateHistoricalBarSizeSeconds(
      const HistorySubscriptionStartPoint &duration) const = 0;
  virtual std::shared_ptr<OrderManager> getOrderManager() = 0;
};
} // namespace midas