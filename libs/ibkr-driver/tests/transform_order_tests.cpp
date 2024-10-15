
#include "CommonDefs.h"
#include "broker-interface/instruments.hpp"
#include "broker-interface/order.hpp"
#include "ibkr/internal/ibkr_order_manager.hpp"
#include "logging/logging.hpp"
#include <atomic>
#include <gtest/gtest.h>
#include <memory>

TEST(IBKRDriverOrderConversion, SimpleOrder) {
  auto logger = std::make_shared<logging::thread_safe_logger_t>(
      logging::create_channel_logger("specs logger"));
  midas::SimpleOrder order(50, midas::OrderDirection::BUY,
                           midas::InstrumentEnum::MicroNasdaqFutures,
                           midas::ExecutionType::Limit, logger, 104.25);
  std::atomic<OrderId> orderIdCtr{0};
  std::list<ibkr::internal::OrderInfo> transformed =
      ibkr::internal::transformOrder(order, orderIdCtr);
  EXPECT_EQ(transformed.size(), 1);
  EXPECT_EQ(orderIdCtr.load(), 1);
  auto &orderInfo = transformed.front();
  EXPECT_EQ(orderInfo.ibkrOrder.orderType, "LMT");
  EXPECT_EQ(orderInfo.ibkrOrder.action, "BUY");
  EXPECT_EQ(orderInfo.ibkrOrder.totalQuantity, 50);
  EXPECT_EQ(orderInfo.ibkrOrder.lmtPrice, 104.25);
  EXPECT_EQ(orderInfo.instrument, midas::InstrumentEnum::MicroNasdaqFutures);
  EXPECT_EQ(orderInfo.ibkrContract.symbol, "MNQ");
}

TEST(IBKRDriverOrderConversion, SimpleStopOrder) {
    auto logger = std::make_shared<logging::thread_safe_logger_t>(
      logging::create_channel_logger("specs logger"));
  midas::SimpleOrder order(50, midas::OrderDirection::SELL,
                           midas::InstrumentEnum::MicroNasdaqFutures,
                           midas::ExecutionType::Stop, logger, 104.25);
  std::atomic<OrderId> orderIdCtr{0};
  std::list<ibkr::internal::OrderInfo> transformed =
      ibkr::internal::transformOrder(order, orderIdCtr);
  EXPECT_EQ(transformed.size(), 1);
  EXPECT_EQ(orderIdCtr.load(), 1);
  auto &orderInfo = transformed.front();
  EXPECT_EQ(orderInfo.ibkrOrder.orderType, "STP");
  EXPECT_EQ(orderInfo.ibkrOrder.action, "SELL");
  EXPECT_EQ(orderInfo.ibkrOrder.totalQuantity, 50);
  EXPECT_EQ(orderInfo.ibkrOrder.lmtPrice, 104.25);
  EXPECT_EQ(orderInfo.instrument, midas::InstrumentEnum::MicroNasdaqFutures);
  EXPECT_EQ(orderInfo.ibkrContract.symbol, "MNQ");
}

TEST(IBKRDriverOrderConversion, BracketBuy) {}

TEST(IBKRDriverOrderConversion, BracketSell) {}