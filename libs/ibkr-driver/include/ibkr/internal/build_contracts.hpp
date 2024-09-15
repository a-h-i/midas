#pragma once
#include "Contract.h"
namespace ibkr::internal {

enum Futures { MNQ };

Contract build_futures_contract(const Futures &future);

} // namespace ibkr::internal