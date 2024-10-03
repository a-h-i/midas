#pragma once
#include "broker-interface/instruments.hpp"
#include "data/bar.hpp"
#include "observers/observers.hpp"
#include <functional>
namespace midas {
/**
 * TODO: Implement
 */
class SubscriptionError;
class Subscription;
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
  Subscription(InstrumentEnum symbol, bool includeTickData);
  Subscription(InstrumentEnum symbol, std::string historyDuration,
               bool includeTickData);
  /**
   * Notifies cancel listeners
   */
  ~Subscription();
  EventSubject<sub_cancel_listener_t> cancelListeners;
  EventSubject<sub_end_listener_t> endListeners;
  EventSubject<sub_error_listener_t> errorListeners;
  EventSubject<sub_bar_listener_t> barListeners;
  /**
   * Only available in realtime mode
   */
  EventSubject<sub_tick_bid_ask_listener_t> askListeners, bidListeners;

  const InstrumentEnum symbol;
  const bool isRealtime, includeTickData;
  const std::optional<std::string> historicalDuration;
};
} // namespace midas