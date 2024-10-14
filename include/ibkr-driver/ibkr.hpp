#pragma once
#include "broker-interface/broker.hpp"
#include "broker-interface/subscription.hpp"
#include <boost/asio/ip/tcp.hpp>
#include <boost/signals2/connection.hpp>
#include <memory>
namespace ibkr {

namespace internal {
class Client;
}
class Driver : public midas::Broker {
public:
  Driver(boost::asio::ip::tcp::endpoint endpoint);
  ~Driver();

  boost::signals2::connection addConnectListener(const std::function<void(bool)> &func);
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
};

} // namespace ibkr