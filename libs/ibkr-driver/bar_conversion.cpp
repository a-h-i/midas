#include "internal/bar_conversion.hpp"
#include "Decimal.h"
#include <boost/date_time/posix_time/conversion.hpp>

midas::Bar ibkr::internal::convertIbkrBar(const Bar &ibkrBar, unsigned int barSizeSeconds) {
  const double wapDouble = DecimalFunctions::decimalToDouble(ibkrBar.wap);
  const double volumeDouble = DecimalFunctions::decimalToDouble(ibkrBar.volume);

  midas::Bar bar(
    barSizeSeconds,
    ibkrBar.count,
    ibkrBar.high,
    ibkrBar.low,
    ibkrBar.open,
    ibkrBar.close,
    wapDouble,
    volumeDouble,
    boost::posix_time::from_time_t(std::stol(ibkrBar.time))
  );

  
  return bar;  
}