#pragma once
#include <boost/date_time/gregorian/greg_date.hpp>
#include <boost/date_time/local_time/local_time.hpp>
#include <memory>
#include <optional>
#include <ostream>
#include <string>

namespace midas {

/**
 * Not all instruments are supported
 */
enum InstrumentEnum {
  MicroNasdaqFutures,
  MicroSPXFutures,
  NVDA,
  TSLA,
  MicroRussel
};

inline std::size_t getDefaultEntryQuantity(InstrumentEnum instrument) {
  switch (instrument) {
  case MicroNasdaqFutures:
  case MicroSPXFutures:
  case MicroRussel:
    return 2;
  case NVDA:
  case TSLA:
    return 100;
  }
  throw std::runtime_error("Unknown instrument enum");
}

enum SupportedCurrencies { USD };

enum SupportedExchanges { COMEX };

inline std::string operator+(const char *lhs, InstrumentEnum instrument) {
  std::string instrumentStr;
  switch (instrument) {

  case MicroNasdaqFutures:
    instrumentStr = "MNQ";
    break;
  case MicroSPXFutures:
    instrumentStr = "MES";
    break;
  case NVDA:
    instrumentStr = "NVDA";
    break;
  case TSLA:
    instrumentStr = "TSLA";
    break;
  case MicroRussel:
    instrumentStr = "M2k";
  }

  return lhs + instrumentStr;
}

inline std::string operator+(const std::string &lhs,
                             InstrumentEnum instrument) {
  return lhs.c_str() + instrument;
}

template <typename stream>
stream &operator<<(stream &s, InstrumentEnum instrument) {
  s << "" + instrument;
  return s;
}

/**
 * The object contains hours (if any)
 * associated with the date
 */
struct TradingHours {
  boost::gregorian::date date;
  std::optional<boost::local_time::local_time_period> regular_period;
  std::optional<boost::local_time::local_time_period> extended_period;
};

template <typename CharT, typename TraitsT>
std::basic_ostream<CharT, TraitsT> &
operator<<(std::basic_ostream<CharT, TraitsT> &stream,
           SupportedCurrencies currency) {
  switch (currency) {
  case USD:
    stream << "USD";
    break;
  }
  return stream;
}

} // namespace midas