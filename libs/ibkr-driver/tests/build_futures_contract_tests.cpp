#include "Contract.h"
#include "broker-interface/instruments.hpp"
#include "ibkr/internal/build_contracts.hpp"
#include <gtest/gtest.h>
#include <regex>

using namespace midas;
using namespace ibkr::internal;
TEST(BuildFuturesContract, lastTradeDate) {
  const Contract contract =
      build_contract(InstrumentEnum::MicroNasdaqFutures);
  std::regex dateRegex("^20\\d\\d[10]\\d$");
  EXPECT_TRUE(
      std::regex_match(contract.lastTradeDateOrContractMonth, dateRegex));
}

class BuildFuturesContractTest : public testing::Test {
protected:


  Contract buildContract(const InstrumentEnum &future, int year, int month,
                         int day) {
    return build_contract(future, [year, month, day]() {
      return boost::gregorian::date(year, month, day);
    }); // Call the function being tested
  }
};

TEST_F(BuildFuturesContractTest, StandardQuarterContract) {
  // Test for a standard case within the current quarter
  Contract contract =
      buildContract(InstrumentEnum::MicroNasdaqFutures, 2024, 1, 15);

  EXPECT_EQ(contract.symbol, "MNQ");
  EXPECT_EQ(contract.exchange, "CME");
  EXPECT_EQ(contract.currency, "USD");
  EXPECT_EQ(contract.secType, "FUT");
  EXPECT_EQ(contract.tradingClass, "MNQ");
  EXPECT_EQ(contract.multiplier, "2");
  EXPECT_EQ(contract.lastTradeDateOrContractMonth, "202403"); // End of Q1
}

TEST_F(BuildFuturesContractTest, SwitchToNextQuarter) {
  // Test for a case where in last month of quarter
  Contract contract =
      buildContract(InstrumentEnum::MicroNasdaqFutures, 2024, 3, 5);

  EXPECT_EQ(contract.symbol, "MNQ");
  EXPECT_EQ(contract.exchange, "CME");
  EXPECT_EQ(contract.currency, "USD");
  EXPECT_EQ(contract.secType, "FUT");
  EXPECT_EQ(contract.tradingClass, "MNQ");
  EXPECT_EQ(contract.multiplier, "2");
  EXPECT_EQ(contract.lastTradeDateOrContractMonth, "202406"); // End of Q2
}

TEST_F(BuildFuturesContractTest, QuarterAndYearRollover) {
  // Test for a case where the quarter and year rolls over
  Contract contract =
      buildContract(InstrumentEnum::MicroNasdaqFutures, 2024, 12, 20);

  EXPECT_EQ(contract.symbol, "MNQ");
  EXPECT_EQ(contract.exchange, "CME");
  EXPECT_EQ(contract.currency, "USD");
  EXPECT_EQ(contract.secType, "FUT");
  EXPECT_EQ(contract.tradingClass, "MNQ");
  EXPECT_EQ(contract.multiplier, "2");
  EXPECT_EQ(contract.lastTradeDateOrContractMonth,
            "202503"); // End of Q1 next year
}

TEST_F(BuildFuturesContractTest, UnsupportedFutureType) {
  // Test for unsupported future type
  EXPECT_THROW(buildContract(static_cast<InstrumentEnum>(-1), 2024, 1, 15),
               std::runtime_error);
}