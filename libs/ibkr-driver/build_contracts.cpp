#include "ibkr/internal/build_contracts.hpp"
#include <exception>
Contract
ibkr::internal::build_futures_contract(const Symbols &future) {
  Contract contract;
  switch (future) {
  case Symbols::MNQ: {
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