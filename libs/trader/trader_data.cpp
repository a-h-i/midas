#include "exceptions/sampling_error.hpp"
#include "trader/trader.hpp"
#include <boost/circular_buffer.hpp>
#include <cstddef>
#include <execution>
#include <numeric>

midas::trader::TraderData::TraderData(std::size_t lookBackSize,
                                      std::size_t candleSizeSeconds,
                                      std::shared_ptr<DataStream> source)
    : lookBackSize(lookBackSize), candleSizeSeconds(candleSizeSeconds),
      downSampleRate(candleSizeSeconds / source->barSizeSeconds),
      lastReadIndex(0), source(source), tradeCounts(lookBackSize),
      highs(lookBackSize), lows(lookBackSize), opens(lookBackSize),
      closes(lookBackSize), vwaps(lookBackSize), volumes(lookBackSize),
      updateListenerId(source->addUpdateListener(
          std::bind(&TraderData::processSource, this))),
      reOrderListenerId(
          source->addReOrderListener(std::bind(&TraderData::clear, this))) {
  if (candleSizeSeconds % source->barSizeSeconds != 0) {
    throw SamplingError(
        "Requested candle size is not divisible by stream bar size");
  }
}

midas::trader::TraderData::~TraderData() {
  source->removeReOrderListener(reOrderListenerId);
  source->removeUpdateListener(updateListenerId);
}

bool midas::trader::TraderData::ok() {
  std::scoped_lock lock(buffersMutex);
  // Depends on the vectors being kept at the same
  return tradeCounts.size() >= lookBackSize;
}

void midas::trader::TraderData::clear() {
  std::scoped_lock lock(buffersMutex);
  tradeCounts.clear();
  highs.clear();
  lows.clear();
  opens.clear();
  closes.clear();
  vwaps.clear();
  volumes.clear();
  timestamps.clear();
  lastReadIndex = 0;
}

enum class ParallelPolicy { unseq, parallel };

/**
 * We need this to dynamically select an execution policy at runtime
 */
template <class Func> auto maybeParallel(Func f, ParallelPolicy policy) {
  switch (policy) {
  case ParallelPolicy::unseq:
    return f(std::execution::unseq);
  default:
    return f(std::execution::par_unseq);
  }
}

void midas::trader::TraderData::processSource() {
  std::scoped_lock lock(buffersMutex);
  const std::ptrdiff_t numCompleteSamples = source->size() / downSampleRate;
  if (numCompleteSamples == 0) {
    return;
  }
  const ParallelPolicy executionPolicy =
      downSampleRate >= 1000 ? ParallelPolicy::parallel : ParallelPolicy::unseq;
  for (; lastReadIndex < source->size(); lastReadIndex += downSampleRate) {

    // we preserve low by getting the min of the range
    const auto lowIterator = maybeParallel(
        [&](auto &pol) {
          return std::min_element(pol, source->lows.begin() + lastReadIndex,
                                  source->lows.begin() + lastReadIndex +
                                      downSampleRate);
        },
        executionPolicy);
    // we preserve high by getting the max of the range
    const auto highIterator = maybeParallel(
        [&](auto &pol) {
          return std::max_element(pol, source->highs.begin() + lastReadIndex,
                                  source->highs.begin() + lastReadIndex +
                                      downSampleRate);
        },
        executionPolicy);
    // trades are just summed
    const auto tradesSum = maybeParallel(
        [&](auto &pol) {
          return std::reduce(pol, source->tradeCounts.begin() + lastReadIndex,
                             source->tradeCounts.begin() + lastReadIndex +
                                 downSampleRate);
        },
        executionPolicy);
    // volume is summed as well
    const auto volumeSum = maybeParallel(
        [&](auto &pol) {
          return std::reduce(pol, source->volumes.begin() + lastReadIndex,
                             source->volumes.begin() + lastReadIndex +
                                 downSampleRate);
        },
        executionPolicy);
    // WAP is more useful for small time periods, we convert into VWAP by
    // multiplying waps by volume and dividing by total volume
    const auto wapSum = std::inner_product(
        source->waps.begin() + lastReadIndex,
        source->waps.begin() + lastReadIndex + downSampleRate,
        source->volumes.begin() + lastReadIndex, 0.0);

    lows.push_back(*lowIterator);
    highs.push_back(*highIterator);
    opens.push_back(source->opens[lastReadIndex]);
    closes.push_back(source->closes[lastReadIndex + downSampleRate - 1]);
    tradeCounts.push_back(tradesSum);
    volumes.push_back(volumeSum);
    vwaps.push_back(wapSum /
                    volumeSum); // divide wapSum by total volume to get vwap
  }
}