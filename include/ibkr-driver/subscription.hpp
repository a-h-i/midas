#pragma once
#include "data/bar.hpp"
#include "known_symbols.hpp"
#include "observers/observers.hpp"
#include <functional>
#include <memory>
namespace ibkr {

/**
 * Manages low level data subscriptions and event notifications.
 */
class Subscription;
class SubscriptionError;
typedef std::function<void(const Subscription &)> sub_cancel_listener_t;
typedef std::function<void(const Subscription &)> sub_end_listener_t;
typedef std::function<void(const Subscription &, const SubscriptionError &)>
    sub_error_listener_t;
typedef std::function<void(const Subscription &, midas::Bar bar)>
    sub_bar_listener_t;
/**
 * Tick by tick listeners
 * TODO: Handle size and optional values
 */
typedef std::function<void(double price)> sub_tick_bid_ask_listener_t; 
typedef std::function<void(double midpoint)> sub_tick_midpoint_listener_t;

class Subscription {
public:
  /**
   * Realtime subscription
   */
  Subscription(Symbols symbol, bool isRealtime, bool includeTickData);

  EventSubject<sub_cancel_listener_t> cancelListeners;
  EventSubject<sub_end_listener_t> endListeners;
  EventSubject<sub_error_listener_t> errorListeners;
  EventSubject<sub_bar_listener_t> barListeners;
  /**
   * Only available in realtime mode
   */
  EventSubject<sub_tick_bid_ask_listener_t> askListeners, bidListeners;
  const Symbols symbol;
  const bool isRealtime, includeTickData;
};

typedef std::weak_ptr<Subscription> subscription_ptr_t;
} // namespace ibkr