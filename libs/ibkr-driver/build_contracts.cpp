#include "ibkr/internal/build_contracts.hpp"
#include <boost/date_time/gregorian/greg_duration_types.hpp>
#include <cstdlib>
#include <format>
#include <functional>
#include <stdexcept>

int calcMonthsOffset(int currentQuarter, int currentMonth) {
  // Find the end month of the current quarter
  int endMonthOfQuarter = currentQuarter * 3;
  if (endMonthOfQuarter < currentMonth) {
    endMonthOfQuarter += 12;
  }
  /*
    the months offset should be the number of months
    to offset to reach the end month of the current quarter
    i.e Mar, Jun, Sep, Dec
   */
  int monthsOffset = endMonthOfQuarter - currentMonth;
  return monthsOffset;
}

Contract ibkr::internal::build_contract(
    const midas::InstrumentEnum &instrument,
    const std::function<boost::gregorian::date()> &getToday) {
  Contract contract;
  auto today = getToday();

  // Determine the current quarter
  int currentMonth = today.month().as_number();
  int currentQuarter = ((currentMonth - 1) / 3) + 1;

  auto contractLastTradeMonth =
      (today +
       boost::gregorian::months(calcMonthsOffset(currentQuarter, currentMonth)))
          .year_month_day();

  if (contractLastTradeMonth.month == currentMonth) {
    currentQuarter = (currentQuarter % 4) + 1;
    contractLastTradeMonth = (today + boost::gregorian::months(calcMonthsOffset(
                                          currentQuarter, currentMonth)))
                                 .year_month_day();
  }

  switch (instrument) {
  case midas::InstrumentEnum::MicroNasdaqFutures: {
    contract.symbol = "MNQ";
    contract.exchange = "CME";
    contract.currency = "USD";
    contract.secType = "FUT";
    contract.tradingClass = "MNQ";
    contract.multiplier = "2";
    contract.lastTradeDateOrContractMonth =
        std::format("{}{:0>2}", static_cast<int>(contractLastTradeMonth.year),
                    contractLastTradeMonth.month.as_number());
    break;
  }
  case midas::InstrumentEnum::MicroSPXFutures: {
    contract.symbol = "MES";
    contract.exchange = "CME";
    contract.currency = "USD";
    contract.secType = "FUT";
    contract.tradingClass = "MES";
    contract.multiplier = "5";
    contract.lastTradeDateOrContractMonth =
        std::format("{}{:0>2}", static_cast<int>(contractLastTradeMonth.year),
                    contractLastTradeMonth.month.as_number());
    break;
  }
  case midas::InstrumentEnum::NVDA: {
    contract.symbol = "NVDA";
    contract.secType = "STK";
    contract.currency = "USD";
    contract.exchange = "SMART";
    break;
  }
  case midas::InstrumentEnum::TSLA: {
    contract.symbol = "TSLA";
    contract.secType = "STK";
    contract.currency = "USD";
    contract.exchange = "SMART";
    break;
  }
  case midas::InstrumentEnum::MicroRussel: {
    contract.symbol = "M2k";
    contract.exchange = "CME";
    contract.currency = "USD";
    contract.secType = "FUT";
    contract.multiplier = "5";
    contract.tradingClass = "M2K";
    break;
  }
  default:
    throw std::runtime_error("Unsupported future: " +
                             std::to_string(static_cast<int>(instrument)));
  }
  return contract;
}