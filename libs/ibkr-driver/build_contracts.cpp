#include "ibkr/internal/build_contracts.hpp"



Contract ibkr::internal::build_futures_contract(IndexFutures future) {
  Contract contract;
  switch(future) {
    case IndexFutures::MNQ:
      contract.symbol = "MNQ";
      contract.exchange = "CME";
      contract.currency = "USD";
      break;
  }
  contract.secType = "CONTFUT";
  return contract;
}