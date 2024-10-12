#pragma once
#include <boost/date_time/posix_time/ptime.hpp>
#include <boost/date_time/posix_time/time_formatters.hpp>
#include <boost/date_time/posix_time/time_parsers.hpp>
#include <ostream>
#include <string>

namespace midas {
struct Bar;
Bar operator>>(const std::string &line, Bar &bar);
struct Bar {
  Bar() = default;
  Bar(unsigned int barSizeSeconds, unsigned int tradeCount, double high,
      double low, double open, double close, double wap, double volume,
      boost::posix_time::ptime utcTime)
      : barSizeSeconds(barSizeSeconds), tradeCount(tradeCount), high(high),
        low(low), open(open), close(close), wap(wap), volume(volume),
        utcTime(utcTime) {}

  unsigned int barSizeSeconds, tradeCount;
  double high, low, open, close, wap, volume;
  boost::posix_time::ptime utcTime;
  template <typename CharT, typename TraitsT>
  friend std::basic_ostream<CharT, TraitsT> &
  operator<<(std::basic_ostream<CharT, TraitsT> &stream, const Bar &bar) {
    stream << boost::posix_time::to_iso_extended_string(bar.utcTime) << ","
           << bar.high << "," << bar.open << "," << bar.close << "," << bar.low
           << "," << bar.volume << "," << bar.tradeCount << "," << bar.wap
           << "," << bar.barSizeSeconds;
    return stream;
  }

  friend midas::Bar midas::operator>>(const std::string &line, Bar &bar);
};
} // namespace midas