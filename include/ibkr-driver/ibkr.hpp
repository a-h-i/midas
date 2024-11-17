#pragma once
#include "CommonDefs.h"
#include "broker-interface/broker.hpp"
#include "broker-interface/subscription.hpp"
#include <atomic>
#include <boost/asio/ip/tcp.hpp>
#include <boost/signals2/connection.hpp>
#include <memory>
namespace ibkr {

namespace internal {
class Client;
class OrderManager;
} // namespace internal
class Driver : public midas::Broker {
  friend internal::OrderManager;

public:
  Driver(boost::asio::ip::tcp::endpoint endpoint);
  ~Driver();

  boost::signals2::connection
  addConnectListener(const std::function<void(bool)> &func);
  virtual bool processCycle() override;
  virtual void
  addSubscription(std::weak_ptr<midas::Subscription> subscription) override;
  /**
   * Does nothing if not connected
   */
  virtual void connect() override;
  /**
   * Does nothing if connected
   */
  virtual void disconnect() override;
  virtual bool isConnected() const override;
  virtual unsigned int estimateHistoricalBarSizeSeconds(
      const midas::HistorySubscriptionStartPoint &duration) const override;

private:
  std::unique_ptr<internal::Client> implementation;
  std::atomic<OrderId> &orderCtr();
};

} // namespace ibkr