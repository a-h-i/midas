#pragma once
#include "data/bar.hpp"
#include "known_symbols.hpp"
#include "observers/observers.hpp"
#include <functional>
#include <memory>
namespace ibkr {

class Subscription;
class SubscriptionError;
typedef std::function<void(const Subscription &)> sub_cancel_listener_t;
typedef std::function<void(const Subscription &)> sub_end_listener_t;
typedef std::function<void(const Subscription &, const SubscriptionError &)>
    sub_error_listener_t;
typedef std::function<void(const Subscription &, midas::Bar bar)>
    sub_bar_listener_t;

class Subscription {
public:
  /**
   * Realtime subscription
   */
  Subscription(Symbols symbol, bool isRealtime);

  EventSubject<sub_cancel_listener_t> cancelListeners;
  EventSubject<sub_end_listener_t> endListeners;
  EventSubject<sub_error_listener_t> errorListeners;
  EventSubject<sub_bar_listener_t> barListeners;

  const Symbols symbol;
  const bool isRealtime;
};

typedef std::weak_ptr<Subscription> subscription_ptr_t;
} // namespace ibkr