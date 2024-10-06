#include "ibkr/internal/bar_conversion.hpp"
#include <boost/date_time/posix_time/conversion.hpp>
#include <gtest/gtest.h>
#include <string>

TEST(IbkrDriverAssertions, BarConversion) {
  Bar internalBar;
  internalBar.close = 22.4;
  internalBar.open = 20;
  internalBar.high = 25;
  internalBar.low = 15;
  internalBar.count = 500;
  internalBar.volume = DecimalFunctions::doubleToDecimal(1000);
  internalBar.wap = DecimalFunctions::doubleToDecimal(40.56);
  internalBar.time = std::to_string(boost::posix_time::to_time_t(
      boost::posix_time::second_clock::universal_time()));
  midas::Bar converted = ibkr::internal::convertIbkrBar(internalBar, 30);
  EXPECT_EQ(converted.barSizeSeconds, 30);
  EXPECT_EQ(converted.close, internalBar.close);
  EXPECT_EQ(converted.open, internalBar.open);
  EXPECT_EQ(converted.high, internalBar.high);
  EXPECT_EQ(converted.low, internalBar.low);
  EXPECT_EQ(converted.tradeCount, internalBar.count);
  EXPECT_EQ(converted.volume, 1000);
  EXPECT_EQ(converted.wap, 40.56);
  EXPECT_EQ(std::to_string(boost::posix_time::to_time_t(converted.utcTime)),
            internalBar.time);
}