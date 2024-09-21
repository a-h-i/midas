#include "ibkr/internal/build_contracts.hpp"
#include <exception>
Contract
ibkr::internal::build_futures_contract(const midas::InstrumentEnum &future) {
  Contract contract;
  switch (future) {
  case midas::InstrumentEnum::MicroNasdaqFutures: {
    contract.symbol = "MNQ";
    contract.exchange = "CME";
    contract.currency = "USD";
    break;
  }
  default:
    throw std::runtime_error("Unsupported future");
  }
  contract.secType = "CONTFUT";
  return contract;
}