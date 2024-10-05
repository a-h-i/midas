#include "backtest/backtest.hpp"
#include "backtest_order_manager.hpp"
#include "broker-interface/broker.hpp"
#include "broker-interface/instruments.hpp"
#include "broker-interface/subscription.hpp"
#include "data/bar.hpp"
#include "data/data_stream.hpp"
#include "logging/logging.hpp"
#include "trader/trader.hpp"
#include <atomic>
#include <list>
#include <memory>

using namespace std::chrono_literals;

static std::unique_ptr<midas::DataStream>
loadHistoricalData(unsigned int barSize, midas::InstrumentEnum instrument,
                   midas::Broker &broker, const midas::HistorySubscriptionStartPoint &duration) {

  std::unique_ptr<midas::DataStream> historicalData(
      new midas::DataStream(barSize));
  std::shared_ptr<midas::Subscription> historicalSubscription =
      std::make_shared<midas::Subscription>(instrument, duration, false);
  historicalSubscription->barListeners.add_listener(
      [&historicalData]([[maybe_unused]] const midas::Subscription &sub,
                        midas::Bar bar) { historicalData->addBars(bar); });
  std::atomic<bool> dataEnded{false};
  historicalSubscription->endListeners.add_listener(
      [&dataEnded]([[maybe_unused]] const midas::Subscription &sub) {
        dataEnded.store(true, std::memory_order::release);
      });
  broker.addSubscription(historicalSubscription);
  while (dataEnded == false) {
    historicalData->waitForData(500ms);
  }
  return historicalData;
}

midas::backtest::BacktestResult midas::backtest::performBacktest(
    InstrumentEnum instrument, BacktestInterval interval,
    std::function<std::unique_ptr<trader::Trader>(
        std::shared_ptr<DataStream>, std::shared_ptr<midas::OrderManager>)>
        traderFactory,
    Broker &broker) {
  std::shared_ptr<logging::thread_safe_logger_t> logger =
      std::make_shared<logging::thread_safe_logger_t>(
          logging::create_channel_logger("backtest " + instrument));
  std::shared_ptr<BacktestOrderManager> orderManager(
      new BacktestOrderManager(logger));
  // now we need to request and wait for historical data.
  constexpr unsigned int historicalBarSize = 30;
  std::unique_ptr<DataStream> historicalData = loadHistoricalData(
      historicalBarSize, instrument, broker, interval.duration);
  // Now that we have the historical data, lets create our dummy real time
  // simulation source and our trader
  std::shared_ptr<DataStream> realtimeSimStream =
      std::make_shared<DataStream>(historicalBarSize);
  auto traderPtr = traderFactory(realtimeSimStream, orderManager);

  // We now enter the simulation phase
  std::list<Bar> barBuffer;
  for (std::size_t barIndex = 0; barIndex < historicalData->size();
       barIndex++) {
    midas::Bar bar(
        historicalData->barSizeSeconds, historicalData->tradeCounts[barIndex],
        historicalData->highs[barIndex], historicalData->lows[barIndex],
        historicalData->opens[barIndex], historicalData->closes[barIndex],
        historicalData->waps[barIndex], historicalData->volumes[barIndex],
        historicalData->timestamps[barIndex]);
    
    if (orderManager->hasActiveOrders()) {
      orderManager->simulate(bar);
      // we don't want to process trades and enter new trades on the same candle.
      barBuffer.push_back(bar);
    } else {
      realtimeSimStream->addBars(barBuffer.begin(), barBuffer.end());
      barBuffer.clear();
      traderPtr->decide();
    }

  }
}