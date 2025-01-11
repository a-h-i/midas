//
// Created by ahi on 1/10/25.
//

#ifndef POSITION_TRACKER_HPP
#define POSITION_TRACKER_HPP
#include "instruments.hpp"
#include <boost/signals2.hpp>
#include <mutex>

namespace midas {

class PositionTracker {
public:
  typedef boost::signals2::signal<void()> realized_pnl_signal_t;

private:
  struct PositionEntry {
    int quantity{0};
    double price{0};
  };
  struct Position {
    std::deque<PositionEntry> shorts, longs;
    double total{0};
  };
  std::unordered_map<InstrumentEnum, Position> positions;
  std::recursive_mutex mutex;
  realized_pnl_signal_t realized_pnl_signal;

public:
  /**
   * Updates realized pnl of instrument
   * @param instrument instrument that execution took place
   * @param quantity negative for sold positive for bought
   * @param pricePerUnit
   */
  void handle_position_update(InstrumentEnum instrument, int quantity,
                              double pricePerUnit);

  std::unordered_map<InstrumentEnum, double> getPnl();
  boost::signals2::connection connectRealizedPnl(const realized_pnl_signal_t::slot_type &slot) {
    return realized_pnl_signal.connect(slot);
  }
};

} // namespace midas

#endif // POSITION_TRACKER_HPP
