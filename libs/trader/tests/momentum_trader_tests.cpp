#include "broker-interface/order.hpp"
#include "data/data_stream.hpp"
#include "trader/momentum_trader.hpp" // Include the MomentumTrader header
#include "gtest/gtest.h"
#include <memory>
#include <vector>

using namespace midas;
using namespace midas::trader;
using namespace std::chrono_literals;

// MockOrderManager derived from OrderManager
class MockOrderManager : public OrderManager {

private:
  std::shared_ptr<Order> lastOrder;
  bool activeOrder;

public:
  MockOrderManager(std::shared_ptr<logging::thread_safe_logger_t> logger)
      : OrderManager(), activeOrder(false) {}

  // Mock implementation of transmit to store the last order
  void transmit(std::shared_ptr<Order> order) override {
    lastOrder = order;
    activeOrder = true;
  }

  // Override hasActiveOrders to reflect if there is an active order
  bool hasActiveOrders() const override { return activeOrder; }

  // Retrieve the last order for testing purposes
  std::optional<Order *> getLastOrder() const {
    return activeOrder ? std::make_optional(lastOrder.get()) : std::nullopt;
  }

  // Reset the mock's active order status
  void clearOrder() {
    lastOrder.reset();
    activeOrder = false;
  }

  std::generator<midas::Order *> getFilledOrders() {
    if (activeOrder && lastOrder) {
      co_yield lastOrder.get();
    }
  }
};

class MomentumTraderTest : public ::testing::Test {
protected:
  std::shared_ptr<MockOrderManager> orderManager;
  std::shared_ptr<DataStream> dataStream;
  std::shared_ptr<logging::thread_safe_logger_t> logger;
  std::unique_ptr<MomentumTrader> trader;

  void SetUp() override {
    logger = std::make_shared<logging::thread_safe_logger_t>(
        logging::create_channel_logger("MomentumTrader Test Logger"));
    orderManager = std::make_shared<MockOrderManager>(logger);
    dataStream = std::make_shared<DataStream>(
        5); // Using 5-second bars for quick testing
    trader = std::make_unique<MomentumTrader>(
        dataStream, orderManager, InstrumentEnum::MicroNasdaqFutures, logger);
  }

  void addMockData(const std::vector<double> &closes,
                   const std::vector<double> &highs,
                   const std::vector<double> &lows,
                   const std::vector<double> &volumes) {
    for (size_t i = 0; i < closes.size(); ++i) {
      Bar bar(5, 10, highs[i], lows[i], closes[i], closes[i], closes[i],
              volumes[i], boost::posix_time::second_clock::universal_time());
      dataStream->addBars(bar);
    }
    dataStream->waitForData(0ms);
  }
};

TEST_F(MomentumTraderTest, BasicEntryCondition) {
  // Set up bullish conditions with sample data
  std::vector<double> closes = {100, 101, 102, 103, 104, 105, 106, 107, 108};
  std::vector<double> highs = {100.5, 101.5, 102.5, 103.5, 104.5,
                               105.5, 106.5, 107.5, 108.5};
  std::vector<double> lows = {99.5,  100.5, 101.5, 102.5, 103.5,
                              104.5, 105.5, 106.5, 107.5};
  std::vector<double> volumes = {500, 500, 500, 500, 500, 500, 500, 500, 500};

  addMockData(closes, highs, lows, volumes);

  // Run the trader's decision logic
  trader->decide();

  auto lastOrder = orderManager->getLastOrder();
  ASSERT_TRUE(lastOrder.has_value()); // Expect an order to have been placed
  EXPECT_EQ(lastOrder.value()->direction, OrderDirection::BUY);
  BracketedOrder *bracket = dynamic_cast<BracketedOrder *>(lastOrder.value());
  ASSERT_TRUE(bracket != nullptr);
  EXPECT_NEAR(bracket->getEntryOrder().targetPrice,
              std::round(closes.back() * 4) / 4,
              0.01); // Verify rounded entry price

  // Clear the active order for the next test
  orderManager->clearOrder();
}

TEST_F(MomentumTraderTest, NoEntryOnBearishCondition) {
  // Set up bearish conditions with sample data
  std::vector<double> closes = {108, 107, 106, 105, 104, 103, 102, 101, 100};
  std::vector<double> highs = {108.5, 107.5, 106.5, 105.5, 104.5,
                               103.5, 102.5, 101.5, 100.5};
  std::vector<double> lows = {107.5, 106.5, 105.5, 104.5, 103.5,
                              102.5, 101.5, 100.5, 99.5};
  std::vector<double> volumes = {500, 500, 500, 500, 500, 500, 500, 500, 500};

  addMockData(closes, highs, lows, volumes);

  // Run the trader's decision logic
  trader->decide();

  auto lastOrder = orderManager->getLastOrder();
  ASSERT_FALSE(lastOrder.has_value()); // No order should be placed
}

TEST_F(MomentumTraderTest, TakeProfitAndStopLossLevels) {
  // Set up data to meet entry conditions
  std::vector<double> closes = {100, 101, 102, 103, 104, 105, 106, 107, 108};
  std::vector<double> highs = {100.5, 101.5, 102.5, 103.5, 104.5,
                               105.5, 106.5, 107.5, 108.5};
  std::vector<double> lows = {99.5,  100.5, 101.5, 102.5, 103.5,
                              104.5, 105.5, 106.5, 107.5};
  std::vector<double> volumes = {500, 500, 500, 500, 500, 500, 500, 500, 500};

  addMockData(closes, highs, lows, volumes);

  trader->decide();

  auto lastOrder = orderManager->getLastOrder();
  ASSERT_TRUE(lastOrder.has_value()); // Expect an order to have been placed

  // Verify that stop loss and take profit were calculated as expected
  double currentAtr = 1.5; // Assuming this matches with test data outcome
  double expectedTakeProfit =
      std::round((closes.back() + 2 * currentAtr) * 4) / 4;
  double expectedStopLoss =
      std::round((closes.back() - 2 * currentAtr) * 4) / 4;

  BracketedOrder *bracket = dynamic_cast<BracketedOrder *>(lastOrder.value());
  ASSERT_TRUE(bracket != nullptr);
  EXPECT_NEAR(bracket->getProfitTakerOrder().targetPrice, expectedTakeProfit,
              0.01);
  EXPECT_NEAR(bracket->getStopOrder().targetPrice, expectedStopLoss, 0.01);

  // Clear the active order for the next test
  orderManager->clearOrder();
}
