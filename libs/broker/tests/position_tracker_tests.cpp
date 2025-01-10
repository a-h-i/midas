//
// Created by ahi on 1/10/25.
//
#include "broker-interface/position_tracker.hpp"
#include <gtest/gtest.h>

TEST(PositionTracking, SimpleStockLong) {

  midas::PositionTracker tracker;
  midas::InstrumentEnum instrument = midas::NVDA;
  tracker.handle_position_update(instrument, 10, 100);
  EXPECT_EQ(0, tracker.getPnl()[midas::NVDA]);  // no realized profit
  tracker.handle_position_update(instrument, -10, 101);
  EXPECT_EQ(10, tracker.getPnl()[midas::NVDA]);

}

TEST(PositionTracking, ComplexStockLong) {
  midas::PositionTracker tracker;
  midas::InstrumentEnum instrument = midas::NVDA;
  tracker.handle_position_update(instrument, 2, 100);
  tracker.handle_position_update(instrument, 1, 103);
  tracker.handle_position_update(instrument, -1, 101);
  EXPECT_EQ(1, tracker.getPnl()[midas::NVDA]);
  tracker.handle_position_update(instrument, -1, 102);
  EXPECT_EQ(3, tracker.getPnl()[midas::NVDA]);
  tracker.handle_position_update(instrument, -1, 102);
  EXPECT_EQ(2, tracker.getPnl()[midas::NVDA]);
}

TEST(PositionTracking, ComplexStockShort) {
  midas::PositionTracker tracker;
  midas::InstrumentEnum instrument = midas::NVDA;
  tracker.handle_position_update(instrument, -100, 100);
  EXPECT_EQ(0, tracker.getPnl()[midas::NVDA]);  // no realized profit
  tracker.handle_position_update(instrument, 50, 101);
  EXPECT_EQ(50, tracker.getPnl()[midas::NVDA]);
  tracker.handle_position_update(instrument, 50, 90);
  EXPECT_EQ(-450, tracker.getPnl()[midas::NVDA]);
}


TEST(PositionTracking, Future) {
  midas::PositionTracker tracker;
  midas::InstrumentEnum instrument = midas::MicroRussel;
  tracker.handle_position_update(instrument, 10, 100);
  EXPECT_EQ(0, tracker.getPnl()[midas::MicroRussel]);  // no realized profit
  tracker.handle_position_update(instrument, -10, 101);
  EXPECT_EQ(10 * 5, tracker.getPnl()[midas::MicroRussel]);
}