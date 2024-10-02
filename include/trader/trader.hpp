#pragma once
#include "broker-interface/instruments.hpp"
#include "broker-interface/order.hpp"
#include "data/data_stream.hpp"
#include <boost/circular_buffer.hpp>
#include <cstddef>
#include <deque>
#include <functional>
#include <memory>
#include <mutex>
#include <utility>

namespace midas::trader {

/**
 * Look back data for traders.
 */
class TraderData {

public: 
  const std::size_t lookBackSize, candleSizeSeconds;
private:
  const std::ptrdiff_t downSampleRate;
  std::size_t lastReadIndex;
  std::shared_ptr<DataStream> source;
  boost::circular_buffer<unsigned int> tradeCounts;
  boost::circular_buffer<double> highs, lows, opens, closes, vwaps, volumes;
  boost::circular_buffer<boost::posix_time::ptime> timestamps;
  const decltype(std::declval<DataStream>().addUpdateListener(
      std::function<void()>())) updateListenerId;
  const decltype(std::declval<DataStream>().addReOrderListener(
      std::function<void()>())) reOrderListenerId;
  std::recursive_mutex buffersMutex;

public:
  /**
   * @param lookBackSize the number of candles to keep
   * @param candleSizeSeconds required candle width, to perform down sampling if
   * needed
   */
  TraderData(std::size_t lookBackSize, std::size_t candleSizeSeconds,
             std::shared_ptr<DataStream> source);
  ~TraderData();
  bool ok();
  operator bool() { return ok(); }
  inline std::size_t size() { return tradeCounts.size(); }
  inline bool empty() { return size() == 0; }

  boost::circular_buffer<double> &closesBuffer() { return closes; }
  boost::circular_buffer<double> &volumesBuffer() { return volumes; }
  boost::circular_buffer<double> &highsBuffer() { return highs; }
  boost::circular_buffer<double> &lowsBuffer() { return lows; }
  /**
   * processes source, can be called manually at start to consume before any
   * updates. Otherwise data is kept in sync via source subscriptions. Note that
   * the same function is used to update on new events
   */
  void processSource();
  /**
   * Clears data. Note that the same function is used to handle re-orfs
   */
  void clear();
};

/**
 * An abstract base class for traders. (trading algorithms)
 * We have decided not to include a member in the base class specifying the
 * associated financial instrument as in the future we may opt for algorithms
 * that trade multiple instruments at the same time.
 *
 *
 */
class Trader {
private:
  std::deque<std::shared_ptr<Order>> currentOrders, executedOrders;

protected:
  TraderData data;
  std::shared_ptr<midas::OrderManager> orderManager;
  std::shared_ptr<logging::thread_safe_logger_t> logger;
  Trader(std::size_t lookBackSize, std::size_t candleSizeSeconds,
         std::shared_ptr<DataStream> source,
         std::shared_ptr<midas::OrderManager> orderManager,
         std::shared_ptr<logging::thread_safe_logger_t> logger)
      : data(lookBackSize, candleSizeSeconds, source),
        orderManager(orderManager), logger(logger) {}

  void enterBracket(InstrumentEnum instrument, unsigned int quantity,
                    OrderDirection direction, double entryPrice,
                    double stopLossPrice, double profitPrice);

public:
  virtual ~Trader() = default;
  virtual void decide() = 0;
  bool hasOpenPosition();
};

/**
 * Designed for fast 2 min momentum exploitation
 */
std::unique_ptr<Trader>
momentumExploit(std::shared_ptr<DataStream> source,
                std::shared_ptr<midas::OrderManager> orderManager,
                InstrumentEnum instrument);
} // namespace midas::trader