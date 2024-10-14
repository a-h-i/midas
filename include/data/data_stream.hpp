#pragma once
#include "bar.hpp"
#include <algorithm>
#include <boost/date_time/posix_time/ptime.hpp>
#include <boost/signals2.hpp>
#include <chrono>
#include <condition_variable>
#include <iterator>
#include <mutex>
#include <vector>

namespace midas {
/**
 * A representation of a data stream for an instrument.
 * Doesn't actually care about the instrument itself, only stores data.
 * The Bars are decomposed into separate vectors
 *
 * Note that a data stream must have uniform bar size.
 * If you need to mix different bar sizes, for example when you want to store
 * historical data along with realtime more granular data. Use two data streams.
 */
class DataStream {
public:
  DataStream(unsigned int barSizeSeconds) : barSizeSeconds(barSizeSeconds) {}
  const unsigned int barSizeSeconds;
  std::vector<unsigned int> tradeCounts;
  std::vector<double> highs, lows, opens, closes, waps, volumes;
  std::vector<boost::posix_time::ptime> timestamps;
  typedef boost::signals2::signal<void()> update_signal_t;
  typedef boost::signals2::signal<void()> reorder_signal_t;

  /**
   * Waits for new data to arrive and processed it.
   * @param timeout max wait time in milliseconds
   * @returns true if there was new data that was processed otherwise it
   * returned due to timeout
   */
  bool waitForData(std::chrono::milliseconds timeout);

  template <typename IteratorT>
    requires std::forward_iterator<IteratorT>
  void addBars(IteratorT begin, IteratorT end) {
    {
      std::lock_guard cvLock(bufferMutex);
      std::copy(begin, end, std::back_inserter(buffer));
    }
    bufferCv.notify_all();
  }
  /**
   * Note that minimal work should be done in the addBars functions
   * As these are invoked by the thread handling messages from the broker,
   * and they should be kept responsive.
   * The purpose of the buffer condition variable is to notify any threads
   * waiting for data so that they can do the processing
   */
  inline void addBars(const midas::Bar &bar) {
    {
      std::lock_guard cvLock(bufferMutex);
      buffer.push_back(bar);
    }
    bufferCv.notify_all();
  }

  /**
   * Called before returning from waitForData
   * to process the new bars.
   * This happens after associated re-order events if any
   * Note that this happens while a lock on the buffers is held.
   * This should be kept in mind as it can block api thread
   */
  boost::signals2::connection
  addUpdateListener(const update_signal_t::slot_type &subscriber);
  /**
   * Called before returning from waitForData
   * to process the new bars.
   * This happens after associated re-order events if any
   * Note that this happens while a lock on the buffers is held.
   * This should be kept in mind as it can block api thread
   */
  boost::signals2::connection
  addReOrderListener(const reorder_signal_t::slot_type &subscriber);


  inline std::size_t size() const { return tradeCounts.size(); }

private:
  std::vector<midas::Bar> buffer
      /**
       * Protects the bar buffer
       * Which is where the API worker produces data.
       * API worker does not interact directly with the data vectors.
       *
       *  */
      ;
  std::mutex bufferMutex;
  std::condition_variable bufferCv;
  reorder_signal_t reorderSignal;
  update_signal_t updateSignal;
};
} // namespace midas