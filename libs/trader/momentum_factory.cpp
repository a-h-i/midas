
#include "trader/momentum_trader.hpp"
#include "trader/trader.hpp"

std::unique_ptr<midas::trader::Trader> midas::trader::momentumExploit(
    std::shared_ptr<midas::DataStream> source,
    std::shared_ptr<midas::OrderManager> orderManager,
    InstrumentEnum instrument) {
  switch (instrument) {
  case InstrumentEnum::TSLA:
  case InstrumentEnum::NVDA:
    return std::make_unique<StockMomentumTrader>(
        source, orderManager, instrument,
        std::make_shared<logging::thread_safe_logger_t>(
            logging::create_channel_logger("Momentum Trader " + instrument)));
  default:
    return std::make_unique<MomentumTrader>(
        source, orderManager, instrument,
        std::make_shared<logging::thread_safe_logger_t>(
            logging::create_channel_logger("Momentum Trader " + instrument)));
  }
}