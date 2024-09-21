#include "ibkr-driver/subscription.hpp"

ibkr::Subscription::Subscription(Symbols symbol, bool realTime, bool includeTickData)
    : symbol(symbol), isRealtime(realTime), includeTickData(includeTickData) {}

