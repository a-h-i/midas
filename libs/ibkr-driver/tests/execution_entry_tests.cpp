#include "Decimal.h"
#include "Execution.h"
#include "broker-interface/order.hpp"
#include "ibkr/internal/order_wrapper.hpp"
#include <gtest/gtest.h>
using namespace ibkr::internal;
TEST(IBKRDriver, ExecutionEntryCorrection) {
  Execution nativeExecution;
  nativeExecution.price = 500;
  nativeExecution.avgPrice = 250;
  nativeExecution.shares = DecimalFunctions::doubleToDecimal(2.0);
  nativeExecution.cumQty = DecimalFunctions::doubleToDecimal(2.0);
  nativeExecution.execId = "145.01";
  nativeExecution.side = "BOT";
  nativeExecution.time = "2011-10-05T14:48:00.000Z";
  Execution nativeCorrection(nativeExecution);
  nativeCorrection.avgPrice = 249;
  nativeCorrection.execId = "145.02";

  ExecutionEntry first(nativeExecution);
  ExecutionEntry correction(nativeCorrection);

  EXPECT_TRUE(correction.corrects(first));
  EXPECT_FALSE(first.corrects(correction));
}

TEST(IBKRDriver, ExecutionEntryParsing) {
  Execution nativeExecution;
  nativeExecution.price = 500;
  nativeExecution.avgPrice = 250;
  nativeExecution.shares = DecimalFunctions::doubleToDecimal(2.0);
  nativeExecution.cumQty = DecimalFunctions::doubleToDecimal(2.0);
  nativeExecution.execId = "145.01";
  nativeExecution.side = "BOT";
  nativeExecution.time = "2011-10-05T14:48:00.000Z";
  ExecutionEntry parsed(nativeExecution);
  EXPECT_EQ(parsed.averagePrice, 250);
  EXPECT_EQ(parsed.totalPrice, 500);
  EXPECT_EQ(parsed.cumulativeQuantity, 2);
  EXPECT_EQ(parsed.quantity, 2);
  EXPECT_EQ(parsed.direction, midas::OrderDirection::BUY);
}