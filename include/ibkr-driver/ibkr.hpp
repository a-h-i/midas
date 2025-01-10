#pragma once
#include "CommonDefs.h"
#include "broker-interface/broker.hpp"
#include "broker-interface/order.hpp"
#include "broker-interface/subscription.hpp"
#include "logging/logging.hpp"
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

private:
  std::shared_ptr<logging::thread_safe_logger_t> logger;
  std::unique_ptr<internal::Client> implementation;
  std::shared_ptr<midas::OrderManager> orderManager;
  std::atomic<OrderId> &orderCtr();

public:
  Driver(boost::asio::ip::tcp::endpoint endpoint);
  ~Driver();

  boost::signals2::connection
  addConnectListener(const std::function<void(bool)> &func);
  bool processCycle() override;
  void
  addSubscription(std::weak_ptr<midas::Subscription> subscription) override;
  /**
   * Does nothing if not connected
   */
  void connect() override;
  /**
   * Does nothing if connected
   */
  void disconnect() override;
  bool isConnected() const override;
  unsigned int estimateHistoricalBarSizeSeconds(
      const midas::HistorySubscriptionStartPoint &duration) const override;
  std::shared_ptr<midas::OrderManager> getOrderManager() override;
};

} // namespace ibkr