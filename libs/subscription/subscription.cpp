#include "broker-interface/subscription.hpp"

midas::Subscription::Subscription(InstrumentEnum symbol, bool realTime,
                                  bool includeTickData)
    : symbol(symbol), isRealtime(realTime), includeTickData(includeTickData) {}
