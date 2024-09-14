#include "midas/instruments.hpp"
#include "midas/timezones.hpp"
#include <exception>

using namespace boost::local_time;
using namespace boost::posix_time;
using namespace boost::gregorian;
midas::Exchange::Exchange(SupportedExchanges exch) : exchange_t(exch) {

  switch (exch) {
  case SupportedExchanges::COMEX: {
    name = "COMEX";
    symbol = "CME";
    currency = SupportedCurrencies::USD;
    tz = central_us_tz;
    break;
  }
  default:
    throw std::runtime_error("Unsupported exchange : " + exch);
  }
}

std::shared_ptr<midas::Exchange>
midas::exchange_from_symbol(SupportedExchanges symbol) {
  return std::shared_ptr<Exchange>(new Exchange(symbol));
}

midas::TradingHours
midas::Exchange::days_trading_hours(const date &date) const {
  if (date.day_of_week() == Saturday) {
    // Always closed on saturdays

    return {date, {}, {}};
  }

  switch (exchange_t) {
  case SupportedExchanges::COMEX: {
    // COMEX exchanges
    const auto extended_hours_start_date =
        date.day() != Sunday ? date - days(1) : date;
    TradingHours trading_hours = {date, {}, {}};
    trading_hours.extended_period = local_time_period(
        local_date_time(extended_hours_start_date, hours(17), tz, local_date_time::EXCEPTION_ON_ERROR),
        time_duration(23, 0, 0));
    if (date.day_of_week() != Sunday) {
      const auto regular_start_time =
          local_date_time(date, time_duration(8, 30, 0), tz,
                          local_date_time::EXCEPTION_ON_ERROR);
      const auto regular_end_time = local_date_time(
          date, hours(17), tz, local_date_time::EXCEPTION_ON_ERROR);
      trading_hours.regular_period = local_time_period(
          regular_start_time, regular_end_time - regular_start_time);
    }
    return trading_hours;
  } break;
  default:
    throw std::runtime_error("Unsupported exchange : " + exchange_name());
  }
}