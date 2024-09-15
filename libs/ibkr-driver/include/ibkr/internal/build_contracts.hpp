#pragma once
#include "Contract.h"
#include "ibkr-driver/known_symbols.hpp"

namespace ibkr::internal {



Contract build_futures_contract(const Symbols &future);

} // namespace ibkr::internal