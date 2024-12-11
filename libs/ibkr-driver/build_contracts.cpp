#include "ibkr/internal/build_contracts.hpp"
#include <boost/date_time/gregorian/greg_duration_types.hpp>
#include <format>
#include <stdexcept>
Contract
ibkr::internal::build_futures_contract(const midas::InstrumentEnum &future) {
  Contract contract;
  auto today = boost::gregorian::day_clock::local_day();

  // Determine the current quarter
  int currentMonth = today.month().as_number();
  const int currentQuarter = ((currentMonth - 1) / 3) + 1;  

  // Find the end month of the current quarter
  const int endMonthOfQuarter = currentQuarter * 3;
  /*
    the months offset should be the number of months 
    to offset to reach the end month of the current quarter 
    i.e Mar, Jun, Sep, Dec
   */
  const int monthsOffset = endMonthOfQuarter - currentMonth;


  auto contractLastTradeMonth =
      (today + boost::gregorian::months(monthsOffset)).year_month_day();

  switch (future) {
  case midas::InstrumentEnum::MicroNasdaqFutures: {
    contract.symbol = "MNQ";
    contract.exchange = "CME";
    contract.currency = "USD";
    contract.secType = "FUT";
    contract.tradingClass = "MNQ";
    contract.multiplier = "2";
    // two months into the future
    contract.lastTradeDateOrContractMonth =
        std::format("{}{:0>2}", static_cast<int>(contractLastTradeMonth.year),
                    contractLastTradeMonth.month.as_number());
    break;
  }
  default:
    throw std::runtime_error("Unsupported future: " + std::to_string(static_cast<int>(future)));
  }
  return contract;
}