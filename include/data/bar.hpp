#pragma once
#include <boost/date_time/posix_time/ptime.hpp>
#include <boost/date_time/posix_time/time_parsers.hpp>
#include <boost/date_time/posix_time/time_formatters.hpp>
#include <istream>
#include <ostream>
#include <ranges>

namespace midas {
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
    stream << boost::posix_time::to_iso_extended_string(bar.utcTime)
           << "," << bar.high << "," << bar.open << ","
           << bar.close << "," << bar.low << ","
           << bar.volume << "," << bar.tradeCount
           << "," << bar.wap;
    return stream;
  }

  
  friend midas::Bar
  operator>>(const std::string &line, Bar &bar) {
    auto splitColumns = std::ranges::split_view(line, std::string(","));
    std::vector<std::string> columns;
    for (auto column : splitColumns) {
      columns.emplace_back(std::string_view(column));
    }
    bar = Bar(30, std::stoi(columns[6]), std::stod(columns[1]),
                   std::stod(columns[4]), std::stod(columns[2]),
                   std::stod(columns[3]), std::stod(columns[7]),
                   std::stod(columns[5]),
                   boost::posix_time::from_iso_extended_string(columns[0])

    );
    return bar;
  }
};
} // namespace midas