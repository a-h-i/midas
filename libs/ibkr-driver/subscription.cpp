#include "ibkr-driver/subscription.hpp"

ibkr::Subscription::Subscription(Symbols symbol, bool realTime)
    : symbol(symbol), isRealtime(realTime) {}

