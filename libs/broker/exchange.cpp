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

local_time_period midas::Exchange::todays_regular_period() const {

  const auto now = local_sec_clock::local_time(tz);
  const auto today = now.date();

  if (today.day() == Saturday) {
    return local_time_period(now, time_duration(0, 0, 0));
  }

  switch (exchange_t) {
  case SupportedExchanges::COMEX: {
    const auto startTime = ptime(today, time_duration(8, 30, 0));
    const auto endTime = ptime(today, hours(16));
    return local_time_period(local_date_time(startTime, tz),
                             endTime - startTime);
  }
  default:
    throw std::runtime_error("Unsupported exchange : " + exchange_name());
  }
}

local_time_period midas::Exchange::todays_extended_period() const {
  const auto now = local_sec_clock::local_time(tz);
  const auto today = now.date();

  if (today.day() == Saturday) {
    return local_time_period(now, time_duration(0, 0, 0));
  }

  switch (exchange_t) {
  case SupportedExchanges::COMEX: {
    auto startDay = today.day() != Sunday ? today - days(1) : today;
    const auto startTime = ptime(startDay, hours(17));
    return local_time_period(local_date_time(startTime, tz),
                             time_duration(23, 0, 0));
  }
  default:
    throw std::runtime_error("Unsupported exchange : " + exchange_name());
  }
}

std::shared_ptr<midas::Exchange>
midas::exchange_from_symbol(SupportedExchanges symbol) {
  return std::shared_ptr<Exchange>(new Exchange(symbol));
}