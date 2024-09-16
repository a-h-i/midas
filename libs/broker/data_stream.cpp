#include "data/data_stream.hpp"

bool midas::DataStream::waitForData(std::chrono::milliseconds timeout) {
  std::vector<midas::Bar> tempBuffer;
  {
    std::unique_lock bufferLock(bufferMutex);
    const bool conditionSatisfied = bufferCv.wait_for(
        bufferLock, timeout, [this] { return !buffer.empty(); });
    if (!conditionSatisfied) {
      // timedout while waiting
      return conditionSatisfied;
    }
    // We copy bars to our temp storage so we can quickly release our lock and
    // not block writer thread
    std::copy(std::cbegin(buffer), std::cend(buffer),
              std::back_inserter(tempBuffer));
    buffer.clear();
  }
  // Now we can take our time computing values
  // First we insert into decomposed vectors
  // Note that it is unknown if we can receive bars out of order or not
  for (auto bar : tempBuffer) {
    const auto timeUpperBound =
        std::upper_bound(timestamps.begin(), timestamps.end(), bar.utcTime);
    const auto insertionDistance =
        std::distance(timestamps.begin(), timestamps.end());

    // Insert data point, they all have the same order
    timestamps.insert(timeUpperBound, bar.utcTime);
    volumes.insert(volumes.begin() + insertionDistance, bar.volume);
    waps.insert(waps.begin() + insertionDistance, bar.wap);
    closes.insert(closes.begin() + insertionDistance, bar.close);
    opens.insert(opens.begin() + insertionDistance, bar.open);
    lows.insert(lows.begin() + insertionDistance, bar.low);
    highs.insert(highs.begin() + insertionDistance, bar.high);
    tradeCounts.insert(tradeCounts.begin() + insertionDistance, bar.tradeCount);
  }
  return true; // we processed data
}