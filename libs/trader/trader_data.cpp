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
      lastReadIndex(0),
      downSampleRate(candleSizeSeconds / source->barSizeSeconds),
      source(source), tradeCounts(lookBackSize), highs(lookBackSize),
      lows(lookBackSize), opens(lookBackSize), closes(lookBackSize),
      vwaps(lookBackSize), volumes(lookBackSize),
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

void midas::trader::TraderData::processSource() {
  std::scoped_lock lock(buffersMutex);
  const std::ptrdiff_t numCompleteSamples = source->size() / downSampleRate;
  if (numCompleteSamples == 0) {
    return;
  }
  for (; lastReadIndex < tradeCounts.size(); lastReadIndex += downSampleRate) {

    // for each candle the high is the first
    const auto lowIterator = std::min_element(
        std::execution::par_unseq, source->lows.begin() + lastReadIndex,
        source->lows.begin() + lastReadIndex + downSampleRate);
    const auto highIterator = std::max_element(
        std::execution::par_unseq, source->highs.begin() + lastReadIndex,
        source->highs.begin() + lastReadIndex + downSampleRate);
    const auto tradesSum = std::reduce(
        std::execution::par_unseq, source->tradeCounts.begin() + lastReadIndex,
        source->tradeCounts.begin() + lastReadIndex + downSampleRate);
    const auto volumeSum = std::reduce(
        std::execution::par_unseq, source->volumes.begin() + lastReadIndex,
        source->volumes.begin() + lastReadIndex + downSampleRate);

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
    vwaps.push_back(wapSum / volumeSum);
  }
}