#include "broker-interface/instruments.hpp"
#include <string>

namespace midas::backtest {

struct BacktestResult {};

struct BacktestInterval {
  std::string duration;
  std::string barSize;
};

namespace literals {
BacktestInterval operator""_years(unsigned long long durationTime) {
  return {.duration = std::to_string(durationTime) + " Y",
          .barSize = "30 secs"};
}
}; // namespace literals

BacktestResult performBacktest(InstrumentEnum instrument, BacktestInterval interval);

}; // namespace midas::backtest