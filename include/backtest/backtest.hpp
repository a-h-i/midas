#include "broker-interface/instruments.hpp"
#include "data/data_stream.hpp"
#include "ibkr-driver/ibkr.hpp"
#include "trader/trader.hpp"
#include <functional>
#include <memory>
#include <string>

namespace midas::backtest {

struct BacktestResult {};

struct BacktestInterval {
  std::string duration;
  std::string barSize;
};

namespace literals {
BacktestInterval operator""_years(unsigned long long durationTime);
}; // namespace literals

BacktestResult performBacktest(
    InstrumentEnum instrument, BacktestInterval interval,
    std::function<std::unique_ptr<trader::Trader>(
        std::shared_ptr<DataStream>, std::shared_ptr<midas::OrderManager>)>
        traderFactory,
    ibkr::Driver &driver);

}; // namespace midas::backtest