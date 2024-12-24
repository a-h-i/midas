#pragma once
#include "Contract.h"
#include "broker-interface/instruments.hpp"

namespace ibkr::internal {

/**
 *
 * @param instrument financial instrument
 * @param getToday function that returns the current day. Parametrized to
 * facilitate testing
 * @return
 */
Contract build_contract(const midas::InstrumentEnum &instrument, const std::function<boost::gregorian::date ()>  &getToday = boost::gregorian::day_clock::local_day);

} // namespace ibkr::internal