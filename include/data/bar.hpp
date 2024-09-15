#pragma once
#include <boost/date_time/posix_time/ptime.hpp>

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
};
} // namespace midas