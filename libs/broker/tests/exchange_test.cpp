#include "boost/date_time/gregorian/gregorian.hpp"
#include "boost/date_time/local_time/local_time.hpp"
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

TEST(ComexTradingHoursTest, ExchangeAssertions) {
  std::shared_ptr<midas::Exchange> comex =
      midas::exchange_from_symbol(midas::SupportedExchanges::COMEX);

  auto saturday = boost::gregorian::date(2024, boost::gregorian::Sep, 14);

  // Saturday Hours
  auto hours = comex->days_trading_hours(saturday);
  EXPECT_FALSE(hours.extended_period.has_value());

  EXPECT_FALSE(hours.regular_period.has_value());
  // Sunday Hours
  hours = comex->days_trading_hours(saturday + boost::gregorian::days(1));
  EXPECT_TRUE(hours.extended_period.has_value());

  EXPECT_FALSE(hours.regular_period.has_value());
  // Work days
  for (int day_offset = 2; day_offset <= 5; day_offset++) {
    const auto offset_date = saturday + boost::gregorian::days(day_offset);
    hours = comex->days_trading_hours(offset_date);
    EXPECT_TRUE(hours.extended_period.has_value());
    EXPECT_TRUE(hours.regular_period.has_value());

    auto expected_regular_start = boost::local_time::local_date_time(
        offset_date, boost::posix_time::time_duration(8, 30, 0),
        comex->timezone(),
        boost::local_time::local_date_time::EXCEPTION_ON_ERROR);
    EXPECT_EQ(hours.regular_period.value().begin(), expected_regular_start);
    auto expected_regular_end = expected_regular_start + boost::posix_time::time_duration(8, 30, 0);
    EXPECT_TRUE(hours.regular_period.value().contains(expected_regular_end - boost::posix_time::millisec(1)));
    EXPECT_FALSE(hours.regular_period.value().contains(expected_regular_end));
  }
}