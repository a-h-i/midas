

#include "gmock/gmock.h"
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <memory>

#include "broker-interface/instruments.hpp"
#include "broker-interface/order.hpp"
#include "logging/logging.hpp"
#include "trader/base_trader.hpp"
using namespace midas;

struct MockOrderManager : public OrderManager {
  MOCK_METHOD(void, transmit, (std::shared_ptr<Order>));
  MOCK_METHOD(std::generator<Order *>, getFilledOrders, ());
  MOCK_METHOD(bool, hasActiveOrders, (), (const));
  MockOrderManager(std::shared_ptr<logging::thread_safe_logger_t> &logger)
      : OrderManager(logger) {}
};

struct MockTrader : public trader::Trader {

  MockTrader(std::shared_ptr<DataStream> source,
             std::shared_ptr<midas::OrderManager> orderManager,
             std::shared_ptr<logging::thread_safe_logger_t> logger)
      : trader::Trader(100, 60, source, orderManager, logger) {}

  MOCK_METHOD(void, handleOrderStatusChangeEvent,
              (Order & order, Order::StatusChangeEvent event));
  MOCK_METHOD(void, decide, ());
  virtual std::string traderName() const override { return "Mock trader"; }

  void enterBracketProxy(InstrumentEnum instrument, unsigned int quantity,
                         OrderDirection direction, double entryPrice,
                         double stopLossPrice, double profitPrice) {
    enterBracket(instrument, quantity, direction, entryPrice, stopLossPrice,
                 profitPrice);
  }
};

class BaseTraderTest : public ::testing::Test {
protected:
  std::shared_ptr<MockOrderManager> orderManager;
  std::shared_ptr<DataStream> dataStream;
  std::shared_ptr<logging::thread_safe_logger_t> logger;
  std::unique_ptr<MockTrader> trader;
  void SetUp() override {
    logger = std::make_shared<logging::thread_safe_logger_t>(
        logging::create_channel_logger("MomentumTrader Test Logger"));
    orderManager = std::make_shared<MockOrderManager>(logger);
    dataStream = std::make_shared<DataStream>(5);
    trader = std::make_unique<MockTrader>(dataStream, orderManager, logger);
  }
};

TEST_F(BaseTraderTest, TraderHasOpenPositionUpdate) {
  EXPECT_FALSE(trader->hasOpenPosition());
  trader->enterBracketProxy(InstrumentEnum::MicroNasdaqFutures, 5,
                            midas::OrderDirection::BUY, 100, 50, 150);
  EXPECT_TRUE(trader->hasOpenPosition());
}