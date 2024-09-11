#pragma once
#include "Contract.h"
#include "ibkr-driver/known_contracts.hpp"
namespace ibkr::internal {

Contract build_futures_contract(ibkr::IndexFutures future);

}