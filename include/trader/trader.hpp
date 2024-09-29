#pragma once
#include "broker-interface/order.hpp"
#include "data/data_stream.hpp"
#include "broker-interface/instruments.hpp"
#include <boost/circular_buffer.hpp>
#include <cstddef>
#include <memory>
#include <mutex>
#include <type_traits>

namespace midas::trader {

/**
 * Look back data for traders.
 */
class TraderData {

  std::size_t lastReadIndex;
  const std::ptrdiff_t downSampleRate;
  std::shared_ptr<DataStream> source;
  boost::circular_buffer<unsigned int> tradeCounts;
  boost::circular_buffer<double> highs, lows, opens, closes, vwaps, volumes;
  boost::circular_buffer<boost::posix_time::ptime> timestamps;
  const std::result_of<decltype (&DataStream::addUpdateListener<void()>)(
      DataStream, void())>::type updateListenerId;
  const std::result_of<decltype (&DataStream::addReOrderListener<void()>)(
      DataStream, void())>::type reOrderListenerId;
  std::recursive_mutex buffersMutex;

public:
  const std::size_t lookBackSize, candleSizeSeconds;
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

class Trader {
protected:
  TraderData data;
  std::shared_ptr<midas::OrderManager> orderManager;
  Trader(std::size_t lookBackSize, std::size_t candleSizeSeconds,
         std::shared_ptr<DataStream> source,
         std::shared_ptr<midas::OrderManager> orderManager)
      : data(lookBackSize, candleSizeSeconds, source),
        orderManager(orderManager) {}

public:
  virtual ~Trader() = default;
  virtual void decide() = 0;
  bool hasOpenPosition();
  void buy(int quantity, InstrumentEnum instrument);
};

/**
 * Designed for fast 2 min momentum exploitation
 */
std::unique_ptr<Trader>
momentumExploit(std::shared_ptr<DataStream> source,
                std::shared_ptr<midas::OrderManager> orderManager, InstrumentEnum instrument);
} // namespace midas::trader