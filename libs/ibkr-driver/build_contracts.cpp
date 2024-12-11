#include "ibkr/internal/build_contracts.hpp"
#include <boost/date_time/gregorian/greg_duration_types.hpp>
#include <format>
#include <stdexcept>
Contract
ibkr::internal::build_futures_contract(const midas::InstrumentEnum &future) {
  Contract contract;
  auto today = boost::gregorian::day_clock::local_day();
  auto contractLastTradeMonth = (today + boost::gregorian::months(2)).year_month_day();

  switch (future) {
  case midas::InstrumentEnum::MicroNasdaqFutures: {
    contract.symbol = "MNQ";
    contract.exchange = "CME";
    contract.currency = "USD";
    contract.secType = "FUT";
    contract.tradingClass = "MNQ";
    contract.multiplier = "2";
    // two months into the future
    contract.lastTradeDateOrContractMonth = std::format("{}{:0>2}",  2025, 3);
    break;
  }
  default:
    throw std::runtime_error("Unsupported future");
  }
  return contract;
}