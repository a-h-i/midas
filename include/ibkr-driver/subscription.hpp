#pragma once
#include "data/bar.hpp"
#include "observers/observers.hpp"
#include "known_symbols.hpp"
#include <boost/date_time/gregorian/gregorian.hpp>
#include <functional>
namespace ibkr {

class Subscription;
class SubscriptionError;
typedef std::function<void(const Subscription &)> sub_cancel_listener_t;
typedef std::function<void(const Subscription &)> sub_end_listener_t;
typedef std::function<void(const Subscription &, const SubscriptionError &)>
    sub_error_listener_t;
typedef std::function<void(const Subscription &, midas::Bar bar)>
    sub_bar_listener_t;

struct Subscription {
  /**
   * Realtime subscription
   */
  Subscription(Symbols symbol);
  Subscription(Symbols symbol, const boost::gregorian::date &startDate);

  EventSubject<sub_cancel_listener_t> cancelListeners;
  EventSubject<sub_end_listener_t> endListeners;
  EventSubject<sub_error_listener_t> errorListeners;
  EventSubject<sub_bar_listener_t> barListeners;


};
} // namespace ibkr