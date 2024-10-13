#include "backtest/backtest.hpp"
#include "backtest_order_manager.hpp"
#include "broker-interface/broker.hpp"
#include "broker-interface/instruments.hpp"
#include "broker-interface/order_summary.hpp"
#include "broker-interface/subscription.hpp"
#include "data/bar.hpp"
#include "data/data_stream.hpp"
#include "data/export.hpp"
#include "logging/logging.hpp"
#include <atomic>
#include <fstream>
#include <ios>
#include <list>
#include <memory>
#include <sstream>

using namespace std::chrono_literals;

static std::shared_ptr<midas::DataStream> fetchHistoricalDataFromRemote(
    unsigned int barSize, midas::InstrumentEnum instrument,
    midas::Broker &broker,
    const midas::HistorySubscriptionStartPoint &duration) {
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
  while (dataEnded.load() == false) {
    historicalData->waitForData(500ms);
  }
  return historicalData;
}

std::string
buildFileName(unsigned int barSize, midas::InstrumentEnum instrument,
              const midas::HistorySubscriptionStartPoint &duration) {
  std::stringstream buffer;
  buffer << instrument << "-B" << barSize << "secs" << "-P_"
         << duration.quantity << to_string(duration.unit) << ".csv";
  return buffer.str();
}

static std::shared_ptr<midas::DataStream>
loadHistoricalData(unsigned int barSize, midas::InstrumentEnum instrument,
                   midas::Broker &broker,
                   const midas::HistorySubscriptionStartPoint &duration,
                   std::shared_ptr<logging::thread_safe_logger_t> logger) {
  const std::string filename = buildFileName(barSize, instrument, duration);
  std::ifstream toRestore(filename, std::ios::in);
  if (toRestore) {
    INFO_LOG(*logger) << "Loading cached data";
    std::shared_ptr<midas::DataStream> restoredStream =
        std::make_shared<midas::DataStream>(barSize);
    toRestore >> *restoredStream;
    return restoredStream;
  };
  INFO_LOG(*logger) << "Fetching remote data";
  std::shared_ptr<midas::DataStream> fetchedStream =
      fetchHistoricalDataFromRemote(barSize, instrument, broker, duration);
  // Now we save it
  std::ofstream toSave(filename, std::ios::out);
  toSave << *fetchedStream;
  return fetchedStream;
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
  const unsigned int historicalBarSize =
      broker.estimateHistoricalBarSizeSeconds(interval.duration);
  INFO_LOG(*logger) << "Fetching historical data";
  std::shared_ptr<DataStream> historicalData = loadHistoricalData(
      historicalBarSize, instrument, broker, interval.duration, logger);
  // Now that we have the historical data, lets create our dummy real time
  // simulation source and our trader
  INFO_LOG(*logger) << "Fetched historical data";
  std::shared_ptr<DataStream> realtimeSimStream =
      std::make_shared<DataStream>(historicalBarSize);
  auto traderPtr = traderFactory(realtimeSimStream, orderManager);

  INFO_LOG(*logger) << "Starting simulation";
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
    barBuffer.push_back(bar);
    if (orderManager->hasActiveOrders()) {
      // we only decide on new orders if we don't have current ones.
      // This is a limitation that should be eventually removed
      orderManager->simulate(&bar);
      // we don't want to process trades and enter new trades on the same
      // candle.

    } else {
      realtimeSimStream->addBars(
          barBuffer.begin(),
          barBuffer.end()); // simulate broker sending data events
      realtimeSimStream->waitForData(0ms); // have the stream process the data
      barBuffer.clear();
      traderPtr->decide();
    }
  }
  INFO_LOG(*logger) << "Simulation complete";
  INFO_LOG(*logger) << "Total orders " << orderManager->totalSize();
  OrderSummaryTracker summaryTracker;

  for (Order *orderPtr : orderManager->getFilledOrders()) {
    summaryTracker.addToSummary(orderPtr);
  }

  BacktestResult result{.summary = summaryTracker.summary(),
                        .originalStream = realtimeSimStream};
  return result;
}