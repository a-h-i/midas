#pragma once
#include "data/data_stream.hpp"
#include <algorithm>
#include <boost/circular_buffer.hpp>
#include <execution>
#include <iterator>

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
  inline void copy(auto &trades, auto &highs, auto &lows, auto &opens,
                   auto &closes, auto &vwaps, auto &volumes, auto &timestamps) {
    std::scoped_lock buffersLock(buffersMutex);
    std::copy(std::execution::unseq, tradeCounts.begin(), tradeCounts.end(),
              std::back_inserter(trades));

    std::copy(std::execution::unseq, this->highs.begin(), this->highs.end(),
              std::back_insert_iterator(highs));
    std::copy(std::execution::unseq, this->lows.begin(), this->lows.end(),
              std::back_inserter(lows));
    std::copy(std::execution::unseq, this->opens.begin(), this->opens.end(),
              std::back_inserter(opens));
    std::copy(std::execution::unseq, this->closes.begin(), this->closes.end(),
              std::back_inserter(closes));
    std::copy(std::execution::unseq, this->vwaps.begin(), this->vwaps.end(),
              std::back_inserter(vwaps));
    std::copy(std::execution::unseq, this->volumes.begin(), this->volumes.end(),
              std::back_inserter(volumes));
    std::copy(std::execution::unseq, this->timestamps.begin(),
              this->timestamps.end(), std::back_inserter(timestamps));
  }
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
} // namespace midas::trader