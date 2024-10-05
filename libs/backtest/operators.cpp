#include "backtest/backtest.hpp"
using namespace midas::backtest;
using namespace midas;

BacktestInterval operator""_years(unsigned long long durationTime) {
  return {.duration = {.unit = SubscriptionDurationUnits::Years,
                       .quantity = durationTime}};
}
