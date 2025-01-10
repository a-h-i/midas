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

    void handle_position_update(InstrumentEnum instrument, int quantity, double pricePerUnit);

    std::unordered_map<InstrumentEnum, double> getPnl();
  };

}


#endif //POSITION_TRACKER_HPP
