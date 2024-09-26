#include "data/bar.hpp"
#include <gtest/gtest.h>
#include <sstream>
#include <string>

TEST(DataUnitAssertions, BarIO) {
  std::stringstream streamSimulator;
  midas::Bar originalBar(30, 100, 500, 333.1, 111.5, 130, 120, 1000,
                         boost::posix_time::second_clock::universal_time());
  streamSimulator << originalBar;
  midas::Bar parsed;
  std::string line;
  streamSimulator >> line;
  line >> parsed;
  EXPECT_EQ(originalBar.barSizeSeconds, parsed.barSizeSeconds);
  EXPECT_DOUBLE_EQ(originalBar.close, parsed.close);
  EXPECT_DOUBLE_EQ(originalBar.low, parsed.low);
  EXPECT_DOUBLE_EQ(originalBar.high, parsed.high);
  EXPECT_DOUBLE_EQ(originalBar.open, parsed.open);
  EXPECT_DOUBLE_EQ(originalBar.volume, parsed.volume);
  EXPECT_DOUBLE_EQ(originalBar.wap, parsed.wap);
  EXPECT_EQ(originalBar.tradeCount, parsed.tradeCount);
  EXPECT_EQ(originalBar.utcTime, parsed.utcTime);
}