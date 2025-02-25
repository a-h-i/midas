#pragma once
#include "data_stream.hpp"
#include <iostream>
#include <istream>
namespace midas {

/**
 * Outputs in CSV style with header.
 * CSV is representation of a candle stick chart. in the following order of
 * columns datetime, high, open, close, low, volume, trades
 */
std::ostream &operator<<(std::ostream &, const DataStream &);
std::istream &operator>>(std::istream &, DataStream &);
} // namespace midas