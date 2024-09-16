#include "data/export.hpp"
#include <boost/date_time/posix_time/time_formatters.hpp>
std::ostream &midas::operator<<(std::ostream &stream, const DataStream &data) {

  // First we write the headers
  stream << "datetime,high,open,close,low,volume,trades";

  for (std::size_t index = 0; index < data.timestamps.size(); index++) {
    stream << "\n"
           << boost::posix_time::to_iso_extended_string(data.timestamps[index])
           << "," << data.highs[index] << "," << data.opens[index] << ","
           << data.closes[index] << "," << data.lows[index] << ","
           << data.volumes[index] << "," << data.tradeCounts[index];
  }

  return stream;
}
