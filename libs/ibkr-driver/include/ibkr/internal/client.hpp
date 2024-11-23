#pragma once

#include "CommissionReport.h"
#include "CommonDefs.h"

#include "EWrapper.h"
#include "active_subscription_state.hpp"
#include "broker-interface/subscription.hpp"
#include "connectivity_state.hpp"
#include "ibkr/internal/order_wrapper.hpp"
#include "logging/logging.hpp"
#include <algorithm>
#include <atomic>
#include <boost/asio.hpp>
#include <boost/signals2.hpp>
#include <boost/unordered/concurrent_flat_map.hpp>
#include <deque>
#include <functional>
#include <iterator>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>
namespace ibkr::internal {

class Client : public EWrapper {

public:
  Client(const boost::asio::ip::tcp::endpoint &endpoint);
  virtual ~Client();
  inline boost::signals2::connection
  addConnectListener(const std::function<void(bool)> &obs) {
    return connectionState.addConnectListener(obs);
  }
  /**
   * Does nothing if not connected
   */
  void connect();
  /**
   * Does nothing if connected
   */
  void disconnect();
  bool isConnected() const;
  inline bool isReady() const { return connectionState.ready(); }
  virtual void nextValidId(OrderId order);
  inline std::atomic<OrderId> &orderCtr() {
    return connectionState.nextValidId;
  }

  /**
   * Needs to be called in a process loop
   */
  bool processCycle();
  void addSubscription(std::weak_ptr<midas::Subscription> subscription);

  /**
   * Requests the transmission of an order
   */
  inline void transmitOrder(std::shared_ptr<NativeOrder> order) {
    std::scoped_lock lock(ordersMutex);
    pendingOrders.push_back(order);
  }
  template <typename Range> inline void transmitOrder(Range &&range) {
    std::scoped_lock lock(ordersMutex);
    std::copy(std::begin(range), std::end(range),
              std::back_inserter(pendingOrders));
  }

#include "ewrapper_decl_and_stubs.hpp"

private:
  ConnectivityState connectionState;
  boost::asio::ip::tcp::endpoint endpoint;

  logging::thread_safe_logger_t logger;
  std::atomic<TickerId> nextTickerId;

  std::vector<std::string> managedAccountIds;
  std::recursive_mutex subscriptionsMutex, commandsMutex, ordersMutex;
  /**
   * Subscriptions that have not yet been processed
   */
  std::deque<std::weak_ptr<midas::Subscription>> pendingSubscriptions;
  std::unordered_map<TickerId, ActiveSubscriptionState> activeSubscriptions;
  std::deque<std::shared_ptr<NativeOrder>> pendingOrders;
  boost::concurrent_flat_map<OrderId, std::shared_ptr<NativeOrder>>
      activeOrders;
  boost::concurrent_flat_map<std::string, CommissionReport>
      unhandledCommissions;
  /**
   * Usually in response to external events,
   * They are processed by the dedicated driver thread in process cycle.
   */
  std::list<std::function<void()>> pendingCommands;

  void processPendingSubscriptions();
  void processPendingCommands();
  void processPendingOrders();
  /**
   * @param func processes function, subscriptionsMutex is locked during
   * invocation. If it returns true subscription is removed.
   * @param ticker ticker id to process
   * @returns number of processed subscriptions
   */
  std::size_t applyToActiveSubscriptions(
      std::function<bool(midas::Subscription &, ActiveSubscriptionState &state)>
          func,
      const TickerId ticker);
  void removeActiveSubscription(const TickerId ticker);

  void handleSubscriptionCancel(const TickerId);

  /**
  @brief Combines executions of an order as they arrive asynchronously and could
  have revisions.
  @note is idempotent
   */
  void handleExecution(const native_execution_t &);
  /**
    @brief combines commission reports of an order as they arrive
    asynchronously. Each one is associated with an execution. There is no set
    order of arrival between Commission reports and executions.
    @note is idempotent
   */
  void handleCommissionReport(const CommissionReport &);

  /**
  @brief checks if the order is completely filled. Then notifies native order.
  @note is idempotent
   */
  void handleOrderCompletelyFilledEvent(OrderId);
};

} // namespace ibkr::internal
