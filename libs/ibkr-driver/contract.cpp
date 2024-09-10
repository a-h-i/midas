#include "Contract.h"
#include "ibkr/internal/client.hpp"
#include "logging/logging.hpp"

void ibkr::internal::Client::contractDetails(
    int reqId, const ContractDetails &contractDetails) {
  DEBUG_LOG(logger) << "Received contract details " << "Symbol "
                    << contractDetails.longName;
}

void ibkr::internal::Client::contractDetailsEnd(int reqId) {
  DEBUG_LOG(logger) << "contractDetailsEnd ";
}
