#include "data/export.hpp"
#include <boost/date_time/posix_time/time_formatters.hpp>
std::ostream &midas::operator<<(std::ostream &stream, const DataStream &data) {

  // First we write the headers
  stream << "datetime,high,open,close,low,volume,trades,wap";

  for (std::size_t index = 0; index < data.timestamps.size(); index++) {
    const midas::Bar bar(
      data.barSizeSeconds,
      data.tradeCounts[index],
      data.highs[index],
      data.lows[index],
      data.opens[index],
      data.closes[index],
      data.waps[index],
      data.volumes[index],
      data.timestamps[index]
    );
    stream << "\n" << bar;
  }

  return stream;
}
