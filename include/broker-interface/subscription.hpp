#pragma once
#include "broker-interface/instruments.hpp"
#include "data/bar.hpp"
#include "exceptions/subscription_error.hpp"
#include <boost/signals2.hpp>
namespace midas {
class Subscription;
typedef boost::signals2::signal<void(const Subscription &)> sub_cancel_signal_t;
typedef boost::signals2::signal<void(const Subscription &)> sub_end_signal_t;
typedef boost::signals2::signal<void(const Subscription &, const SubscriptionError &)>
    sub_error_signal_t;
typedef boost::signals2::signal<void(const Subscription &, midas::Bar bar)>
    sub_bar_signal_t;


enum class SubscriptionDurationUnits {
  Years,
  Months,
};

inline std::string to_string(SubscriptionDurationUnits unit) {
  switch (unit) {
  case SubscriptionDurationUnits::Years:
    return "Years";
  case SubscriptionDurationUnits::Months:
    return "Months";
  }
  
}

struct HistorySubscriptionStartPoint {
  SubscriptionDurationUnits unit;
  unsigned int quantity;
};

/**
 * Tick by tick listeners
 * TODO: Handle size and optional values
 */

class Subscription {
public:
  Subscription(InstrumentEnum symbol, bool includeTickData);
  Subscription(InstrumentEnum symbol,
               HistorySubscriptionStartPoint historyDuration,
               bool includeTickData);
  /**
   * Notifies cancel listeners
   */
  ~Subscription();

  const InstrumentEnum symbol;
  const bool isRealtime, includeTickData;
  const std::optional<HistorySubscriptionStartPoint> historicalDuration;
  sub_cancel_signal_t cancelSignal;
  sub_end_signal_t endSignal;
  sub_error_signal_t errorSignal;
  sub_bar_signal_t barSignal;
};
} // namespace midas