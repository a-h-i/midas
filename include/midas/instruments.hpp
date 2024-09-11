#pragma once
#include "boost/date_time/local_time/local_time.hpp"
#include <memory>
#include <ostream>
#include <string>

namespace midas {

/**
 * Not all instruments are supported
 */
enum SupportedInstruments { MicroNasdaqFutures };

enum InstrumentType { IndexFuture };

enum SupportedCurrencies { USD };

enum SupportedExchanges { COMEX };

class Exchange {

public:
  inline boost::local_time::time_zone_ptr timezone() const { return tz; };

  inline SupportedCurrencies supported_currency() const { return currency; };
  inline const std::string &exchange_name() const { return name; }
  inline const std::string &exchange_symbol() const { return symbol; }
  boost::local_time::local_time_period todays_regular_period() const;
  boost::local_time::local_time_period todays_extended_period() const;
  /**
   * Singleton factory
   */
  friend std::shared_ptr<Exchange> exchange_from_symbol(SupportedExchanges symbol);
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
  return stream << "[Exchange: " << exchange.exchange_name << " - "
                << exchange.exchange_symbol << " - "
                << exchange.supported_currency << "]";
}

/**
 * A representation of a financial instrument.
 * Contains static information about the instrument itself
 */
struct FinancialInstrument {

  std::string name;
  InstrumentType sub_type;
  SupportedInstruments symbol;
  std::shared_ptr<const Exchange> exchange;
  virtual ~FinancialInstrument() = default;
};


std::shared_ptr<Exchange> exchange_from_symbol(SupportedExchanges symbol);
} // namespace midas