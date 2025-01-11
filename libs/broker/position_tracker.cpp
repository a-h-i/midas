//
// Created by ahi on 1/10/25.
//
#include "broker-interface/position_tracker.hpp"

static double getInstrumentMultiplier(midas::InstrumentEnum instrument) {
  switch (instrument) {

  case midas::MicroNasdaqFutures:
    return 2;
    break;
  case midas::MicroSPXFutures:
    return 5;
    break;
  case midas::NVDA:
    return 1;
    break;
  case midas::TSLA:
    return 1;
    break;
  case midas::MicroRussel:
    return 5;
    break;
  }
  throw std::runtime_error("invalid instrument");
}

void midas::PositionTracker::handle_position_update(InstrumentEnum instrument,
                                                    int quantity,
                                                    double price) {
  std::scoped_lock lock(mutex);
  auto &position = positions[instrument];
  double profitAccum{position.total};
  if (quantity < 0) {

    while (!position.longs.empty() && quantity != 0) {
      auto diff = std::min(std::abs(quantity), position.longs.front().quantity);
      auto priceDiff = price - position.longs.front().price;
      profitAccum += priceDiff * diff * getInstrumentMultiplier(instrument);
      position.longs[0].quantity -= std::abs(quantity);
      if (position.longs[0].quantity == 0) {
        position.longs.pop_front();
      }
      quantity += diff;
    }
    if (quantity != 0) {
      position.shorts.push_back({.quantity = quantity, .price = price});
    }
  } else {
    while (!position.shorts.empty() && quantity != 0) {
      auto diff = std::min(quantity, std::abs(position.shorts.front().quantity));
      auto priceDiff = price - position.shorts.front().price;
      profitAccum += priceDiff * diff * getInstrumentMultiplier(instrument);
      position.shorts[0].quantity -= std::abs(quantity);
      if (position.shorts[0].quantity == 0) {
        position.shorts.pop_front();
      }
      quantity -= diff;
    }
    if (quantity != 0) {
      position.longs.push_back({.quantity = quantity, .price = price});
    }
  }
  position.total = profitAccum;
  realized_pnl_signal();
}

std::unordered_map<midas::InstrumentEnum, double>
midas::PositionTracker::getPnl() {
  std::scoped_lock lock(mutex);
  std::unordered_map<midas::InstrumentEnum, double> realized;

  for (auto &[instrument, position] : positions) {
    realized[instrument] = position.total;
  }
  return realized;
}

