#pragma once
#include <boost/date_time/posix_time/ptime.hpp>

#include <ostream>
namespace midas {
struct Bar {
  Bar(unsigned int barSizeSeconds, unsigned int tradeCount, double high,
      double low, double open, double close, double wap, double volume,
      boost::posix_time::ptime utcTime)
      : barSizeSeconds(barSizeSeconds), tradeCount(tradeCount), high(high),
        low(low), open(open), close(close), wap(wap), volume(volume), utcTime(utcTime) {}

  const unsigned int barSizeSeconds, tradeCount;
  const double high, low, open, close, wap, volume;
  const boost::posix_time::ptime utcTime;
  template <typename CharT, typename TraitsT>
  friend std::basic_ostream<CharT, TraitsT> &operator<<(std::basic_ostream<CharT, TraitsT> &stream, const Bar &bar) {
    stream << "[BAR - time: " << bar.utcTime << " high: " << bar.high << " low: " << bar.low
    << " open: " << bar.open << " close: " << bar.close << " wap:  " << bar.wap << " volume: " << bar.volume
    << " trades: " << bar.tradeCount << " barSize: " << bar.barSizeSeconds <<  " seconds";
    return stream;
  }

};
} // namespace midas