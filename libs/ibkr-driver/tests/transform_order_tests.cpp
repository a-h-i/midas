
#include "CommonDefs.h"
#include "broker-interface/instruments.hpp"
#include "broker-interface/order.hpp"
#include "ibkr/internal/ibkr_order_manager.hpp"
#include "logging/logging.hpp"
#include <algorithm>
#include <atomic>
#include <gtest/gtest.h>
#include <iterator>
#include <memory>

TEST(IBKRDriverOrderConversion, SimpleOrder) {
  auto logger = std::make_shared<logging::thread_safe_logger_t>(
      logging::create_channel_logger("specs logger"));
  midas::SimpleOrder order(50, midas::OrderDirection::BUY,
                           midas::InstrumentEnum::MicroNasdaqFutures,
                           midas::ExecutionType::Limit, logger, 104.25);
  std::atomic<OrderId> orderIdCtr{0};
  std::list<ibkr::internal::NativeOrder> transformed =
      ibkr::internal::transformOrder(order, orderIdCtr);
  EXPECT_EQ(transformed.size(), 1);
  EXPECT_EQ(orderIdCtr.load(), 1);
  auto &orderInfo = transformed.front();
  EXPECT_EQ(orderInfo.nativeOrder.orderType, "LMT");
  EXPECT_EQ(orderInfo.nativeOrder.action, "BUY");
  EXPECT_EQ(orderInfo.nativeOrder.totalQuantity, 50);
  EXPECT_EQ(orderInfo.nativeOrder.lmtPrice, 104.25);
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
  std::list<ibkr::internal::NativeOrder> transformed =
      ibkr::internal::transformOrder(order, orderIdCtr);
  EXPECT_EQ(transformed.size(), 1);
  EXPECT_EQ(orderIdCtr.load(), 1);
  auto &orderInfo = transformed.front();
  EXPECT_EQ(orderInfo.nativeOrder.orderType, "STP");
  EXPECT_EQ(orderInfo.nativeOrder.action, "SELL");
  EXPECT_EQ(orderInfo.nativeOrder.totalQuantity, 50);
  EXPECT_EQ(orderInfo.nativeOrder.lmtPrice, 104.25);
  EXPECT_EQ(orderInfo.instrument, midas::InstrumentEnum::MicroNasdaqFutures);
  EXPECT_EQ(orderInfo.ibkrContract.symbol, "MNQ");
}

TEST(IBKRDriverOrderConversion, BracketBuy) {
  auto logger = std::make_shared<logging::thread_safe_logger_t>(
      logging::create_channel_logger("specs logger"));
  midas::BracketedOrder order(50, midas::OrderDirection::BUY,
                              midas::InstrumentEnum::MicroNasdaqFutures, 20, 30,
                              10, logger);
  std::atomic<OrderId> orderIdCtr{0};
  std::list<ibkr::internal::NativeOrder> transformed =
      ibkr::internal::transformOrder(order, orderIdCtr);
  EXPECT_EQ(transformed.size(), 3);
  EXPECT_EQ(orderIdCtr.load(), 3);
  auto &parent = transformed.front();

  bool hasAssociatedParent = std::ranges::all_of(
      std::next(transformed.begin()), transformed.end(),
      [&parent](ibkr::internal::NativeOrder &other) {
        return other.nativeOrder.parentId == parent.nativeOrder.orderId;
      });
  EXPECT_TRUE(hasAssociatedParent);
  bool notTransmitted =
      std::ranges::none_of(transformed.begin(), std::prev(transformed.end()),
                           [](ibkr::internal::NativeOrder &info) {
                             return info.nativeOrder.transmit;
                           });
  EXPECT_TRUE(notTransmitted);
  EXPECT_TRUE(
      transformed.back()
          .nativeOrder.transmit); // only last order has transmit flag set

  bool correctInstrument =
      std::ranges::all_of(transformed, [](ibkr::internal::NativeOrder &info) {
        return info.instrument == midas::InstrumentEnum::MicroNasdaqFutures &&
               info.ibkrContract.symbol == "MNQ";
      });
  EXPECT_TRUE(correctInstrument);

  EXPECT_EQ(parent.nativeOrder.orderType, "LMT");
  EXPECT_EQ(parent.nativeOrder.lmtPrice, 20);
  EXPECT_EQ(parent.nativeOrder.totalQuantity, 50);
  EXPECT_EQ(parent.nativeOrder.action, "BUY");
  auto itr = transformed.begin();
  std::advance(itr, 1);
  auto &profit = *itr;
  EXPECT_EQ(profit.nativeOrder.orderType, "LMT");
  EXPECT_EQ(profit.nativeOrder.lmtPrice, 30);
  EXPECT_EQ(profit.nativeOrder.totalQuantity, 50);
  EXPECT_EQ(profit.nativeOrder.action, "SELL");

  std::advance(itr, 1);
  auto &stop = *itr;
  EXPECT_EQ(stop.nativeOrder.orderType, "STP");
  EXPECT_EQ(stop.nativeOrder.lmtPrice, 10);
  EXPECT_EQ(stop.nativeOrder.totalQuantity, 50);
  EXPECT_EQ(stop.nativeOrder.action, "SELL");
}

TEST(IBKRDriverOrderConversion, BracketSell) {
  auto logger = std::make_shared<logging::thread_safe_logger_t>(
      logging::create_channel_logger("specs logger"));
  midas::BracketedOrder order(50, midas::OrderDirection::SELL,
                              midas::InstrumentEnum::MicroNasdaqFutures, 20, 10,
                              30, logger);
  std::atomic<OrderId> orderIdCtr{0};
  std::list<ibkr::internal::NativeOrder> transformed =
      ibkr::internal::transformOrder(order, orderIdCtr);
  EXPECT_EQ(transformed.size(), 3);
  EXPECT_EQ(orderIdCtr.load(), 3);
  auto &parent = transformed.front();

  bool hasAssociatedParent = std::ranges::all_of(
      std::next(transformed.begin()), transformed.end(),
      [&parent](ibkr::internal::NativeOrder &other) {
        return other.nativeOrder.parentId == parent.nativeOrder.orderId;
      });
  EXPECT_TRUE(hasAssociatedParent);
  bool notTransmitted =
      std::ranges::none_of(transformed.begin(), std::prev(transformed.end()),
                           [](ibkr::internal::NativeOrder &info) {
                             return info.nativeOrder.transmit;
                           });
  EXPECT_TRUE(notTransmitted);
  EXPECT_TRUE(
      transformed.back()
          .nativeOrder.transmit); // only last order has transmit flag set

  bool correctInstrument =
      std::ranges::all_of(transformed, [](ibkr::internal::NativeOrder &info) {
        return info.instrument == midas::InstrumentEnum::MicroNasdaqFutures &&
               info.ibkrContract.symbol == "MNQ";
      });
  EXPECT_TRUE(correctInstrument);

  EXPECT_EQ(parent.nativeOrder.orderType, "LMT");
  EXPECT_EQ(parent.nativeOrder.lmtPrice, 20);
  EXPECT_EQ(parent.nativeOrder.totalQuantity, 50);
  EXPECT_EQ(parent.nativeOrder.action, "SELL");
  auto itr = transformed.begin();
  std::advance(itr, 1);
  auto &profit = *itr;
  EXPECT_EQ(profit.nativeOrder.orderType, "LMT");
  EXPECT_EQ(profit.nativeOrder.lmtPrice, 10);
  EXPECT_EQ(profit.nativeOrder.totalQuantity, 50);
  EXPECT_EQ(profit.nativeOrder.action, "BUY");

  std::advance(itr, 1);
  auto &stop = *itr;
  EXPECT_EQ(stop.nativeOrder.orderType, "STP");
  EXPECT_EQ(stop.nativeOrder.lmtPrice, 30);
  EXPECT_EQ(stop.nativeOrder.totalQuantity, 50);
  EXPECT_EQ(stop.nativeOrder.action, "BUY");
}