#pragma once
#include "bar.h"
#include "data/bar.hpp"

namespace ibkr::internal {
  /**
   * Note that an assumption is made that bar time string is utc epoch timestamps
   */
  midas::Bar convertIbkrBar(const Bar &ibkrBar, unsigned int barSizeSeconds);
}