#pragma once
#include "Contract.h"
#include "midas/instruments.hpp"

namespace ibkr::internal {



Contract build_futures_contract(const midas::InstrumentEnum &future);

} // namespace ibkr::internal