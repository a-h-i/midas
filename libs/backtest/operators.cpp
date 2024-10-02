#include "backtest/backtest.hpp"
using namespace midas::backtest;

BacktestInterval operator""_years(unsigned long long durationTime) {
  return {.duration = std::to_string(durationTime) + " Y",
          .barSize = "30 secs"};
}
