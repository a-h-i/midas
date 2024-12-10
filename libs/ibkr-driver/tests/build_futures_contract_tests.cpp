#include "Contract.h"
#include "broker-interface/instruments.hpp"
#include "ibkr/internal/build_contracts.hpp"
#include <gtest/gtest.h>
#include <regex>

TEST(BuildFuturesContract, lastTradeDate) {
  const Contract contract = ibkr::internal::build_futures_contract(
      midas::InstrumentEnum::MicroNasdaqFutures);
  std::regex dateRegex("^20\\d\\d[10]\\d$");
  EXPECT_TRUE(std::regex_match(contract.lastTradeDateOrContractMonth, dateRegex));
}