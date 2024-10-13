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
enum InstrumentEnum { MicroNasdaqFutures };

enum InstrumentType { IndexFuture };

enum SupportedCurrencies { USD };

enum SupportedExchanges { COMEX };

inline std::string operator+(const char *lhs, InstrumentEnum instrument) {
  std::string instrumentStr;
  switch (instrument) {

  case MicroNasdaqFutures:
    instrumentStr = "MNQ";
    break;
  }
  return lhs + instrumentStr;
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

class Exchange {

public:
  inline boost::local_time::time_zone_ptr timezone() const { return tz; };

  inline SupportedCurrencies supported_currency() const { return currency; };
  inline const std::string &exchange_name() const { return name; }
  inline const std::string &exchange_symbol() const { return symbol; }
  /**
   * Returns the trading hours (if any) for the date specified
   */
  TradingHours days_trading_hours(const boost::gregorian::date &date) const;
  /**
   * Singleton factory
   */
  friend std::shared_ptr<Exchange>
  exchange_from_symbol(SupportedExchanges symbol);
  const SupportedExchanges exchange_t;

private:
  Exchange(SupportedExchanges exch);
  boost::local_time::time_zone_ptr tz;
  std::string name;
  std::string symbol;

  SupportedCurrencies currency;
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

template <typename CharT, typename TraitsT>
std::basic_ostream<CharT, TraitsT> &
operator<<(std::basic_ostream<CharT, TraitsT> &stream, Exchange exchange) {
  return stream << "[Exchange: " << exchange.exchange_name() << " - "
                << exchange.exchange_symbol() << " - "
                << exchange.supported_currency() << "]";
}

/**
 * A representation of a financial instrument.
 * Contains static information about the instrument itself
 */
struct FinancialInstrument {

  std::string name;
  InstrumentType sub_type;
  InstrumentEnum symbol;
  std::shared_ptr<const Exchange> exchange;
  virtual ~FinancialInstrument() = default;
};

std::shared_ptr<Exchange> exchange_from_symbol(SupportedExchanges symbol);
} // namespace midas