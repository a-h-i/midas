#include "broker-interface/subscription.hpp"

midas::Subscription::Subscription(InstrumentEnum symbol,
                                  bool includeTickData)
    : symbol(symbol), isRealtime(true), includeTickData(includeTickData) {}

midas::Subscription::Subscription(InstrumentEnum symbol,
                                  HistorySubscriptionStartPoint historicalDuration,
                                  bool includeTickData)
    : symbol(symbol), isRealtime(false), includeTickData(includeTickData), historicalDuration(historicalDuration) {}

midas::Subscription::~Subscription() {
  cancelListeners.notify(*this);
}