#include "exceptions/sampling_error.hpp"
#include "trader/trader.hpp"
#include <algorithm>
#include <boost/circular_buffer.hpp>
#include <cmath>
#include <cstddef>
#include <execution>

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
template <typename T> class SampleWrapper {
public:
  class Iterator {
  public:
    using value_type = T *;
    using difference_type = std::ptrdiff_t;
    using pointer = T *;
    using reference = value_type;
    Iterator() : ptr(nullptr) {}
    Iterator(pointer ptr) : ptr(ptr) {}
    reference operator*() const { return ptr; }
    pointer operator->() { return ptr; }

    // Prefix increment
    Iterator &operator++() {
      ptr += 3;
      return *this;
    }

    // Postfix increment
    Iterator operator++(int) {
      Iterator tmp = *this;
      ++(*this);
      return tmp;
    }

    friend bool operator==(const Iterator &a, const Iterator &b) {
      return a.ptr == b.ptr;
    };
    friend bool operator!=(const Iterator &a, const Iterator &b) {
      return a.ptr != b.ptr;
    };

  private:
    pointer ptr;
    SampleWrapper *wrapper;
  };
  SampleWrapper(Iterator::difference_type sampleWidth,
                Iterator::difference_type numberOfSamples,
                Iterator::pointer start)
      : sampleWidth(sampleWidth), numberOfSamples(numberOfSamples),
        start(start) {}

  Iterator begin() { return Iterator(start); }
  Iterator end() {
    typename Iterator::difference_type offset = sampleWidth * numberOfSamples;
    return Iterator(start + offset);
  }

private:
  const Iterator::difference_type sampleWidth, numberOfSamples;
  const Iterator::pointer start;
};

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
  const std::ptrdiff_t downSampleRate = candleSizeSeconds / source->barSizeSeconds;
  const std::ptrdiff_t numCompleteSamples = source->size() / downSampleRate;
  if (numCompleteSamples == 0) {
    return;
  }
  SampleWrapper<unsigned int> tradeCountsSample(
      downSampleRate, numCompleteSamples, source->tradeCounts.data());
  static_assert(std::forward_iterator<SampleWrapper<unsigned int>::Iterator>);

  std::for_each(std::execution::par_unseq, tradeCountsSample.begin(),
                tradeCountsSample.end(),
                [this, downSampleRate]([[maybe_unused]]const unsigned int *range) {

                });
}