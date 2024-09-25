#include "exceptions/sampling_error.hpp"
#include "trader/trader.hpp"
#include <boost/circular_buffer.hpp>
#include <cstddef>
#include <numeric>

midas::trader::TraderData::TraderData(std::size_t lookBackSize,
                                      std::size_t candleSizeSeconds,
                                      std::shared_ptr<DataStream> source)
    : lookBackSize(lookBackSize), candleSizeSeconds(candleSizeSeconds),
      lastReadIndex(0), source(source), tradeCounts(lookBackSize),
      highs(lookBackSize), lows(lookBackSize), opens(lookBackSize),
      closes(lookBackSize), waps(lookBackSize), volumes(lookBackSize),
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
  return tradeCounts.size() > lookBackSize;
}

template <typename Container, typename SourceContainer>
void downSample(Container &destinationContainer,
                const SourceContainer &sourceContainer,
                std::ptrdiff_t downSampleRate, std::size_t lastReadIndex) {
  for (; lastReadIndex < sourceContainer.size();
       lastReadIndex += downSampleRate) {
    typename Container::value_type sum =
        std::reduce(&sourceContainer[lastReadIndex],
                    &sourceContainer[lastReadIndex + downSampleRate]);
    destinationContainer.push_back(sum / downSampleRate);
  }
}

void midas::trader::TraderData::clear() {
  std::scoped_lock lock(buffersMutex);
  tradeCounts.clear();
  highs.clear();
  lows.clear();
  opens.clear();
  closes.clear();
  waps.clear();
  volumes.clear();
  timestamps.clear();
  lastReadIndex = 0;
}

void midas::trader::TraderData::processSource() {
  std::scoped_lock lock(buffersMutex);
  if (!ok()) {
    return;
  }
  const std::ptrdiff_t downSampleRate =
      candleSizeSeconds / source->barSizeSeconds;
  const std::ptrdiff_t numCompleteSamples = source->size() / downSampleRate;
  if (numCompleteSamples == 0) {
    return;
  }
  const std::size_t oldSize = tradeCounts.size();
  downSample(tradeCounts, source->tradeCounts, downSampleRate, lastReadIndex);
  downSample(highs, source->highs, downSampleRate, lastReadIndex);
  downSample(lows, source->lows, downSampleRate, lastReadIndex);
  downSample(opens, source->opens, downSampleRate, lastReadIndex);
  downSample(closes, source->closes, downSampleRate, lastReadIndex);
  downSample(waps, source->waps, downSampleRate, lastReadIndex);
  downSample(volumes, source->volumes, downSampleRate, lastReadIndex);
  // update last read index
  lastReadIndex += (tradeCounts.size() - oldSize) * downSampleRate;
}