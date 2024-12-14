#include "broker-interface/broker.hpp"
#include "broker-interface/instruments.hpp"
#include "broker-interface/subscription.hpp"
#include "data/data_stream.hpp"
#include "trader/trader.hpp"
#include <functional>
#include <memory>

namespace midas::backtest {

struct BacktestResult {
  TradeSummary summary;
  std::shared_ptr<DataStream> originalStream;
  std::string orderDetails;
};


struct BacktestInterval {
  /**
   * Intervals are from duration to now
   */
  HistorySubscriptionStartPoint duration;
};

namespace literals {
BacktestInterval operator""_years(unsigned long long durationTime);
BacktestInterval operator""_months(unsigned long long durationTime);
}; // namespace literals

BacktestResult performBacktest(
    InstrumentEnum instrument, BacktestInterval interval,
    std::function<std::unique_ptr<trader::Trader>(
        std::shared_ptr<DataStream>, std::shared_ptr<midas::OrderManager>)>
        traderFactory,
    Broker &broker);

}; // namespace midas::backtest