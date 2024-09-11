#include "midas/instruments.hpp"
#include <gtest/gtest.h>

TEST(ComexTest, ExchangeAssertions) {
  std::shared_ptr<midas::Exchange> comex =
      midas::exchange_from_symbol(midas::SupportedExchanges::COMEX);
  EXPECT_EQ(comex->exchange_name(), "COMEX");
  EXPECT_EQ(comex->supported_currency(), midas::SupportedCurrencies::USD);
  auto tz = comex->timezone();
  EXPECT_EQ(tz->base_utc_offset(), boost::posix_time::time_duration(-5, 0, 0));
  EXPECT_TRUE(tz->has_dst());
  EXPECT_EQ(tz->dst_offset(), boost::posix_time::time_duration(-1, 0, 0));
  EXPECT_EQ(tz->std_zone_abbrev(), "CST");
  EXPECT_EQ(tz->dst_zone_abbrev(), "CDT");
}
