

#include "gmock/gmock.h"
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <memory>
#include <vector>

#include "broker-interface/instruments.hpp"
#include "broker-interface/order.hpp"
#include "logging/logging.hpp"
#include "trader/base_trader.hpp"
using namespace midas;

struct MockOrderManager : public OrderManager {
  std::vector<std::shared_ptr<BracketedOrder>> activeOrders;

  MOCK_METHOD(std::list<Order *>, getFilledOrders, ());
  MOCK_METHOD(bool, hasActiveOrders, (), (const));
  MockOrderManager(std::shared_ptr<logging::thread_safe_logger_t> &logger)
      : OrderManager(logger) {}

  virtual void transmit(std::shared_ptr<midas::Order> order) override {
    order->setTransmitted();
  }
};

struct MockTrader : public trader::Trader {
  std::vector<std::shared_ptr<BracketedOrder>> brackets;
  MockTrader(std::shared_ptr<DataStream> source,
             std::shared_ptr<midas::OrderManager> orderManager,
             std::shared_ptr<logging::thread_safe_logger_t> logger)
      : trader::Trader(100, 60, source, orderManager, logger) {}

  MOCK_METHOD(void, decide, ());
  virtual std::string traderName() const override { return "Mock trader"; }

  void enterBracketProxy(InstrumentEnum instrument, unsigned int quantity,
                         OrderDirection direction, double entryPrice,
                         double stopLossPrice, double profitPrice) {
    enterBracket(instrument, quantity, direction, entryPrice, stopLossPrice,
                 profitPrice);
  }

  void enterBracket(InstrumentEnum instrument, unsigned int quantity,
                    OrderDirection direction, double entryPrice,
                    double stopLossPrice, double profitPrice) {
    std::shared_ptr<BracketedOrder> createdOrder = std::make_shared<BracketedOrder>(
            quantity, direction, instrument, entryPrice, profitPrice,
            stopLossPrice, logger);
        brackets.emplace_back(createdOrder);
     Trader::enterBracket(createdOrder);
  }
  void fillOrders(bool atProfit) {
    for (auto &order : brackets) {
      order->getEntryOrder().setFilled(100, 1, order->requestedQuantity);
      auto & subOrder = atProfit ? order->getProfitTakerOrder() : order->getStopOrder();
      subOrder.setFilled(atProfit ? 150 : 50, 1, order->requestedQuantity);
    }
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

TEST_F(BaseTraderTest, TraderHasOpenPositionUpdateAtProfit) {
  EXPECT_FALSE(trader->hasOpenPosition());
  trader->enterBracketProxy(InstrumentEnum::MicroNasdaqFutures, 5,
                            midas::OrderDirection::BUY, 100, 50, 150);
  EXPECT_TRUE(trader->hasOpenPosition());
  trader->fillOrders(true);
  EXPECT_FALSE(trader->hasOpenPosition());
}

TEST_F(BaseTraderTest, TraderHasOpenPositionUpdateAtLoss) {
  EXPECT_FALSE(trader->hasOpenPosition());
  trader->enterBracketProxy(InstrumentEnum::MicroNasdaqFutures, 5,
                            midas::OrderDirection::BUY, 100, 50, 150);
  EXPECT_TRUE(trader->hasOpenPosition());
  trader->fillOrders(false);
  EXPECT_FALSE(trader->hasOpenPosition());
}