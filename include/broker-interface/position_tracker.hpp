//
// Created by ahi on 1/10/25.
//

#ifndef POSITION_TRACKER_HPP
#define POSITION_TRACKER_HPP
#include "instruments.hpp"

#include <mutex>

namespace midas {

class PositionTracker {
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
};

} // namespace midas

#endif // POSITION_TRACKER_HPP
