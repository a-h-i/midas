#include "backtest/backtest.hpp"
using namespace midas::backtest;
using namespace midas;

BacktestInterval literals::operator""_years(unsigned long long durationTime) {
  return {.duration = {.unit = SubscriptionDurationUnits::Years,
                       .quantity = static_cast<unsigned int>(durationTime)}};
}



BacktestInterval literals::operator""_months(unsigned long long durationTime) {
  return {.duration = {.unit = SubscriptionDurationUnits::Months,
                       .quantity = static_cast<unsigned int>(durationTime)}};
}