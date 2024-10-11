#include "backtest_order_manager.hpp"
#include "broker-interface/instruments.hpp"
#include "broker-interface/order.hpp"
#include "data/bar.hpp"

#include "gmock/gmock.h"
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <memory>

using testing::_;
using testing::MockFunction;
using namespace midas;
using namespace backtest;

class SimulationOrderTransmitterTest : public testing::Test {
protected:
  Bar belowTriggerPriceBar{
      30, 500, 99,
      45, 55,  70,
      69, 69,  boost::posix_time::second_clock::universal_time()};
  Bar aboveTriggerPriceBar{
      30,  500, 205,
      101, 107, 205,
      69,  69,  boost::posix_time::second_clock::universal_time()};
  Bar overlappingTriggerPriceBar{
      30, 500, 205,
      98, 200, 205,
      69, 69,  boost::posix_time::second_clock::universal_time()};

  Bar overlappingBracketStop{
      30, 500, 104,
      45, 60,  100,
      69, 69,  boost::posix_time::second_clock::universal_time()};

  Bar belowBracketStop{
      30, 500, 30,
      10, 15,  25,
      69, 69,  boost::posix_time::second_clock::universal_time()};

  Bar aboveBracketStop{
      30, 500, 85,
      60, 60,  79,
      69, 69,  boost::posix_time::second_clock::universal_time()};

  Bar overlappingBracketLimit{
      30,  500, 160,
      100, 60,  100,
      69,  69,  boost::posix_time::second_clock::universal_time()};

  Bar belowBracketLimit{
      30,  500, 105,
      100, 100, 100,
      69,  69,  boost::posix_time::second_clock::universal_time()};

  Bar aboveBracketLimit{
      30,  500, 200,
      180, 190, 195,
      69,  69,  boost::posix_time::second_clock::universal_time()};

  std::unique_ptr<Order> createSimpleOrder(OrderDirection direction,
                                           ExecutionType execution) {
    std::shared_ptr<logging::thread_safe_logger_t> logger =
        std::make_shared<logging::thread_safe_logger_t>(
            logging::create_channel_logger("test"));
    auto ptr = std::unique_ptr<Order>(
        new SimpleOrder(100, direction, InstrumentEnum::MicroNasdaqFutures,
                        execution, logger, 100));
    ptr->setTransmitted();
    return ptr;
  }

  std::unique_ptr<Order> createBracketedOrder(OrderDirection direction) {
    std::shared_ptr<logging::thread_safe_logger_t> logger =
        std::make_shared<logging::thread_safe_logger_t>(
            logging::create_channel_logger("test"));
    if (direction == OrderDirection::BUY) {
      auto ptr = std::unique_ptr<Order>(
          new BracketedOrder(10, direction, InstrumentEnum::MicroNasdaqFutures,
                             100, 150, 50, logger));
      ptr->setTransmitted();
      return ptr;
    } else {
      auto ptr = std::unique_ptr<Order>(
          new BracketedOrder(10, direction, InstrumentEnum::MicroNasdaqFutures,
                             100, 50, 150, logger));
      ptr->setTransmitted();
      return ptr;
    }
  }

  std::unique_ptr<Order> createHoldingBracketedOrder(OrderDirection direction) {
    std::shared_ptr<logging::thread_safe_logger_t> logger =
        std::make_shared<logging::thread_safe_logger_t>(
            logging::create_channel_logger("test"));
    auto ptr = createBracketedOrder(direction);
    dynamic_cast<BracketedOrder *>(ptr.get())->getEntryOrder().setFilled(
        100, 0.25 * 10, 10);
    return ptr;
  }
};

TEST_F(SimulationOrderTransmitterTest, SimpleBuyStopOrderBelow) {

  MockFunction<void(double, double, bool)> mockCallback;
  auto buyStopOrder =
      createSimpleOrder(OrderDirection::BUY, ExecutionType::Stop);
  // A buy stop order is a stop loss for a short position.
  // Thus it should trigger when price is higher than target.

  EXPECT_CALL(mockCallback, Call(_, _, _)).Times(0);

  SimulationOrderTransmitter transmitter(&belowTriggerPriceBar,
                                         mockCallback.AsStdFunction());

  buyStopOrder->visit(transmitter);
}

TEST_F(SimulationOrderTransmitterTest, SimpleBuyStopOrderOverlap) {

  MockFunction<void(double, double, bool)> mockCallback;
  auto buyStopOrder =
      createSimpleOrder(OrderDirection::BUY, ExecutionType::Stop);
  // A buy stop order is a stop loss for a short position.
  // Thus it should trigger when price is higher than target.

  EXPECT_CALL(mockCallback, Call(100, _, true)).Times(1);

  SimulationOrderTransmitter transmitter(&overlappingTriggerPriceBar,
                                         mockCallback.AsStdFunction());

  buyStopOrder->visit(transmitter);
}

TEST_F(SimulationOrderTransmitterTest, SimpleBuyStopOrderAbove) {

  MockFunction<void(double, double, bool)> mockCallback;
  auto buyStopOrder =
      createSimpleOrder(OrderDirection::BUY, ExecutionType::Stop);
  // A buy stop order is a stop loss for a short position.
  // Thus it should trigger when price is higher than target.

  EXPECT_CALL(mockCallback, Call(100, _, true)).Times(1);

  SimulationOrderTransmitter transmitter(&aboveTriggerPriceBar,
                                         mockCallback.AsStdFunction());

  buyStopOrder->visit(transmitter);
}

TEST_F(SimulationOrderTransmitterTest, SimpleSellStopOrderAbove) {
  MockFunction<void(double, double, bool)> mockCallback;
  auto sellStopOrder =
      createSimpleOrder(OrderDirection::SELL, ExecutionType::Stop);
  EXPECT_CALL(mockCallback, Call(_, _, _)).Times(0);
  SimulationOrderTransmitter transmitter(&aboveTriggerPriceBar,
                                         mockCallback.AsStdFunction());

  sellStopOrder->visit(transmitter);
}

TEST_F(SimulationOrderTransmitterTest, SimpleSellStopOrderOverlap) {
  MockFunction<void(double, double, bool)> mockCallback;
  auto sellStopOrder =
      createSimpleOrder(OrderDirection::SELL, ExecutionType::Stop);
  EXPECT_CALL(mockCallback, Call(100, _, true)).Times(1);
  SimulationOrderTransmitter transmitter(&overlappingTriggerPriceBar,
                                         mockCallback.AsStdFunction());

  sellStopOrder->visit(transmitter);
}

TEST_F(SimulationOrderTransmitterTest, SimpleSellStopOrderBelow) {
  MockFunction<void(double, double, bool)> mockCallback;
  auto sellStopOrder =
      createSimpleOrder(OrderDirection::SELL, ExecutionType::Stop);
  EXPECT_CALL(mockCallback, Call(100, _, true)).Times(1);
  SimulationOrderTransmitter transmitter(&belowTriggerPriceBar,
                                         mockCallback.AsStdFunction());

  sellStopOrder->visit(transmitter);
}

TEST_F(SimulationOrderTransmitterTest, SimpleLimitBuyOrderAbove) {
  // We are buying, meaning we enter the position if the price is below or equal
  auto order = createSimpleOrder(OrderDirection::BUY, ExecutionType::Limit);
  MockFunction<void(double, double, bool)> mockCallback;
  EXPECT_CALL(mockCallback, Call(_, _, _)).Times(0);
  SimulationOrderTransmitter transmitter(&aboveTriggerPriceBar,
                                         mockCallback.AsStdFunction());
  order->visit(transmitter);
}

TEST_F(SimulationOrderTransmitterTest, SimpleLimitBuyOrderOverlap) {
  // We are buying, meaning we enter the position if the price is below or equal
  auto order = createSimpleOrder(OrderDirection::BUY, ExecutionType::Limit);
  MockFunction<void(double, double, bool)> mockCallback;
  EXPECT_CALL(mockCallback, Call(100, _, true)).Times(1);
  SimulationOrderTransmitter transmitter(&overlappingTriggerPriceBar,
                                         mockCallback.AsStdFunction());
  order->visit(transmitter);
}

TEST_F(SimulationOrderTransmitterTest, SimpleLimitBuyOrderBelow) {
  // We are buying, meaning we enter the position if the price is below or equal
  auto order = createSimpleOrder(OrderDirection::BUY, ExecutionType::Limit);
  MockFunction<void(double, double, bool)> mockCallback;
  EXPECT_CALL(mockCallback, Call(100, _, true)).Times(1);
  SimulationOrderTransmitter transmitter(&belowTriggerPriceBar,
                                         mockCallback.AsStdFunction());
  order->visit(transmitter);
}

TEST_F(SimulationOrderTransmitterTest, SimpleLimitSellOrderAbove) {
  // we are selling, so we exit the position if the price is above or equal
  auto order = createSimpleOrder(OrderDirection::SELL, ExecutionType::Limit);
  MockFunction<void(double, double, bool)> mockCallback;
  EXPECT_CALL(mockCallback, Call(100, _, true)).Times(1);
  SimulationOrderTransmitter transmitter(&aboveTriggerPriceBar,
                                         mockCallback.AsStdFunction());
  order->visit(transmitter);
}

TEST_F(SimulationOrderTransmitterTest, SimpleLimitSellOrderBelow) {
  // we are selling, so we exit the position if the price is above or equal
  // we are selling, so we exit the position if the price is above or equal
  auto order = createSimpleOrder(OrderDirection::SELL, ExecutionType::Limit);
  MockFunction<void(double, double, bool)> mockCallback;
  EXPECT_CALL(mockCallback, Call(_, _, _)).Times(0);
  SimulationOrderTransmitter transmitter(&belowTriggerPriceBar,
                                         mockCallback.AsStdFunction());
  order->visit(transmitter);
}

TEST_F(SimulationOrderTransmitterTest, SimpleLimitSellOrderOverlap) {
  // we are selling, so we exit the position if the price is above or equal
  // we are selling, so we exit the position if the price is above or equal
  auto order = createSimpleOrder(OrderDirection::SELL, ExecutionType::Limit);
  MockFunction<void(double, double, bool)> mockCallback;
  EXPECT_CALL(mockCallback, Call(100, _, true)).Times(1);
  SimulationOrderTransmitter transmitter(&overlappingTriggerPriceBar,
                                         mockCallback.AsStdFunction());
  order->visit(transmitter);
}

TEST_F(SimulationOrderTransmitterTest, BracketedBuyOrderEntryOverlap) {

  auto order = createBracketedOrder(OrderDirection::BUY);
  MockFunction<void(double, double, bool)> mockCallback;
  EXPECT_CALL(mockCallback, Call(100, _, false)).Times(1);
  // We are buying, meaning we enter the position if the price is below or equal
  SimulationOrderTransmitter transmitter(&overlappingTriggerPriceBar,
                                         mockCallback.AsStdFunction());
  order->visit(transmitter);
}

TEST_F(SimulationOrderTransmitterTest, BracketedBuyOrderEntryBelow) {

  auto order = createBracketedOrder(OrderDirection::BUY);
  MockFunction<void(double, double, bool)> mockCallback;
  EXPECT_CALL(mockCallback, Call(100, _, false)).Times(1);
  // We are buying, meaning we enter the position if the price is below or equal
  SimulationOrderTransmitter transmitter(&belowTriggerPriceBar,
                                         mockCallback.AsStdFunction());
  order->visit(transmitter);
}

TEST_F(SimulationOrderTransmitterTest, BracketedBuyOrderEntryAbove) {

  auto order = createBracketedOrder(OrderDirection::BUY);
  MockFunction<void(double, double, bool)> mockCallback;
  EXPECT_CALL(mockCallback, Call(_, _, _)).Times(0);
  // We are buying, meaning we enter the position if the price is below or equal
  SimulationOrderTransmitter transmitter(&aboveTriggerPriceBar,
                                         mockCallback.AsStdFunction());
  order->visit(transmitter);
}

TEST_F(SimulationOrderTransmitterTest, BracketedBuyOrderStopBelow) {

  auto order = createHoldingBracketedOrder(OrderDirection::BUY);
  MockFunction<void(double, double, bool)> mockCallback;
  EXPECT_CALL(mockCallback, Call(50, _, true)).Times(1);
  SimulationOrderTransmitter transmitter(&belowBracketStop,
                                         mockCallback.AsStdFunction());
  order->visit(transmitter);
}

TEST_F(SimulationOrderTransmitterTest, BracketedBuyOrderStopAbove) {

  auto order = createHoldingBracketedOrder(OrderDirection::BUY);
  MockFunction<void(double, double, bool)> mockCallback;
  EXPECT_CALL(mockCallback, Call(_, _, _)).Times(0);
  SimulationOrderTransmitter transmitter(&aboveBracketStop,
                                         mockCallback.AsStdFunction());
  order->visit(transmitter);
}

TEST_F(SimulationOrderTransmitterTest, BracketedBuyOrderStopOverlap) {

  auto order = createHoldingBracketedOrder(OrderDirection::BUY);
  MockFunction<void(double, double, bool)> mockCallback;
  EXPECT_CALL(mockCallback, Call(50, _, true)).Times(1);

  SimulationOrderTransmitter transmitter(&overlappingBracketStop,
                                         mockCallback.AsStdFunction());
  order->visit(transmitter);
}

TEST_F(SimulationOrderTransmitterTest, BracketedBuyOrderLimitAbove) {

  auto order = createHoldingBracketedOrder(OrderDirection::BUY);
  MockFunction<void(double, double, bool)> mockCallback;
  EXPECT_CALL(mockCallback, Call(150, _, true)).Times(1);

  SimulationOrderTransmitter transmitter(&aboveBracketLimit,
                                         mockCallback.AsStdFunction());
  order->visit(transmitter);
}

TEST_F(SimulationOrderTransmitterTest, BracketedBuyOrderLimitBelow) {

  auto order = createHoldingBracketedOrder(OrderDirection::BUY);
  MockFunction<void(double, double, bool)> mockCallback;
  EXPECT_CALL(mockCallback, Call(_, _, _)).Times(0);

  SimulationOrderTransmitter transmitter(&belowBracketLimit,
                                         mockCallback.AsStdFunction());
  order->visit(transmitter);
}

TEST_F(SimulationOrderTransmitterTest, BracketedBuyOrderLimitOverlap) {

  auto order = createHoldingBracketedOrder(OrderDirection::BUY);
  MockFunction<void(double, double, bool)> mockCallback;
  EXPECT_CALL(mockCallback, Call(150, _, true)).Times(1);

  SimulationOrderTransmitter transmitter(&overlappingBracketLimit,
                                         mockCallback.AsStdFunction());
  order->visit(transmitter);
}

TEST_F(SimulationOrderTransmitterTest, BracketedSellOrderEntryOverlap) {
  auto order = createBracketedOrder(OrderDirection::SELL);
  MockFunction<void(double, double, bool)> mockCallback;
  EXPECT_CALL(mockCallback, Call(100, _, false)).Times(1);
  SimulationOrderTransmitter transmitter(&overlappingTriggerPriceBar,
                                         mockCallback.AsStdFunction());
  order->visit(transmitter);
}

TEST_F(SimulationOrderTransmitterTest, BracketedSellOrderEntryBelow) {
  auto order = createBracketedOrder(OrderDirection::SELL);
  MockFunction<void(double, double, bool)> mockCallback;
  EXPECT_CALL(mockCallback, Call(_, _, _)).Times(0);
  SimulationOrderTransmitter transmitter(&belowTriggerPriceBar,
                                         mockCallback.AsStdFunction());
  order->visit(transmitter);
}

TEST_F(SimulationOrderTransmitterTest, BracketedSellOrderEntryAbove) {
  auto order = createBracketedOrder(OrderDirection::SELL);
  MockFunction<void(double, double, bool)> mockCallback;
  EXPECT_CALL(mockCallback, Call(100, _, false)).Times(1);
  SimulationOrderTransmitter transmitter(&aboveTriggerPriceBar,
                                         mockCallback.AsStdFunction());
  order->visit(transmitter);
}

TEST_F(SimulationOrderTransmitterTest, BracketedSellOrderStopBelow) {
  auto order = createHoldingBracketedOrder(OrderDirection::SELL);
  MockFunction<void(double, double, bool)> mockCallback;
  EXPECT_CALL(mockCallback, Call(_, _, _)).Times(0);
  SimulationOrderTransmitter transmitter(&belowBracketLimit,
                                         mockCallback.AsStdFunction());
  order->visit(transmitter);
}

TEST_F(SimulationOrderTransmitterTest, BracketedSellOrderStopAbove) {
  auto order = createHoldingBracketedOrder(OrderDirection::SELL);
  MockFunction<void(double, double, bool)> mockCallback;
  EXPECT_CALL(mockCallback, Call(150, _, true)).Times(1);
  SimulationOrderTransmitter transmitter(&aboveBracketLimit,
                                         mockCallback.AsStdFunction());
  order->visit(transmitter);
}

TEST_F(SimulationOrderTransmitterTest, BracketedSellOrderStopOverlap) {
  auto order = createHoldingBracketedOrder(OrderDirection::SELL);
  MockFunction<void(double, double, bool)> mockCallback;
  EXPECT_CALL(mockCallback, Call(150, _, true)).Times(1);
  SimulationOrderTransmitter transmitter(&overlappingBracketLimit,
                                         mockCallback.AsStdFunction());
  order->visit(transmitter);
}

TEST_F(SimulationOrderTransmitterTest, BracketedSellOrderLimitBelow) {
  auto order = createHoldingBracketedOrder(OrderDirection::SELL);
  MockFunction<void(double, double, bool)> mockCallback;
  EXPECT_CALL(mockCallback, Call(50, _, true)).Times(1);
  SimulationOrderTransmitter transmitter(&belowBracketStop,
                                         mockCallback.AsStdFunction());
  order->visit(transmitter);
}

TEST_F(SimulationOrderTransmitterTest, BracketedSellOrderLimitAbove) {
  auto order = createHoldingBracketedOrder(OrderDirection::SELL);
  MockFunction<void(double, double, bool)> mockCallback;
  EXPECT_CALL(mockCallback, Call(_, _, _)).Times(0);
  SimulationOrderTransmitter transmitter(&aboveBracketStop,
                                         mockCallback.AsStdFunction());
  order->visit(transmitter);
}

TEST_F(SimulationOrderTransmitterTest, BracketedSellOrderLimitOverlap) {
  auto order = createHoldingBracketedOrder(OrderDirection::SELL);
  MockFunction<void(double, double, bool)> mockCallback;
  EXPECT_CALL(mockCallback, Call(50, _, true)).Times(1);
  SimulationOrderTransmitter transmitter(&overlappingBracketStop,
                                         mockCallback.AsStdFunction());
  order->visit(transmitter);
}
