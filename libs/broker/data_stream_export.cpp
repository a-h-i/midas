#include "data/export.hpp"
#include <boost/date_time/posix_time/time_formatters.hpp>
#include <string>
using namespace std::chrono_literals;
std::ostream &midas::operator<<(std::ostream &stream, const DataStream &data) {

  // First we write the headers
  stream << "datetime,high,open,close,low,volume,trades,wap,barSizeSeconds";

  for (std::size_t index = 0; index < data.timestamps.size(); index++) {
    const midas::Bar bar(data.barSizeSeconds, data.tradeCounts[index],
                         data.highs[index], data.lows[index], data.opens[index],
                         data.closes[index], data.waps[index],
                         data.volumes[index], data.timestamps[index]);
    stream << "\n" << bar;
  }

  return stream;
}

std::istream &midas::operator>>(std::istream &stream, DataStream &data) {
  // skip headers
  std::string line;
  std::getline(stream, line);
  while (std::getline(stream, line)) {
    Bar bar;
    line >> bar;
    data.addBars(bar);
  }
  data.waitForData(0ms);
  return stream;
}
