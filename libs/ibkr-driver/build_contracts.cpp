#include "ibkr/internal/build_contracts.hpp"
#include <stdexcept>
Contract
ibkr::internal::build_futures_contract(const midas::InstrumentEnum &future) {
  Contract contract;
  switch (future) {
  case midas::InstrumentEnum::MicroNasdaqFutures: {
    contract.symbol = "MNQ";
    contract.exchange = "CME";
    contract.currency = "USD";
    contract.secType = "FUT";
    contract.tradingClass = "MNQ";
    contract.multiplier = "2";
    contract.lastTradeDateOrContractMonth = "202412";
    break;
  }
  default:
    throw std::runtime_error("Unsupported future");
  }
  return contract;
}