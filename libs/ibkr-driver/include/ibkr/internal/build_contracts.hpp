#pragma once
#include "Contract.h"
#include "broker-interface/instruments.hpp"

namespace ibkr::internal {



Contract build_futures_contract(const midas::InstrumentEnum &future, const std::function<boost::gregorian::date ()>  &getToday = boost::gregorian::day_clock::local_day);

} // namespace ibkr::internal