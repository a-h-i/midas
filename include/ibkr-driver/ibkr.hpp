#pragma once
#include "broker-interface/subscription.hpp"
#include <boost/asio/ip/tcp.hpp>
#include <memory>
namespace ibkr {

namespace internal {
class Client;
}
class Driver {
public:
  Driver(boost::asio::ip::tcp::endpoint endpoint);
  ~Driver();
  

  void addConnectListener(const std::function<void()> &func);
  bool processCycle();
  void addSubscription(std::weak_ptr<midas::Subscription> subscription);
   /**
   * Does nothing if not connected
   */
  void connect();
  /**
   * Does nothing if connected
   */
  void disconnect();
  bool isConnected() const;

private:
  std::unique_ptr<internal::Client> implementation;
};

} // namespace ibkr