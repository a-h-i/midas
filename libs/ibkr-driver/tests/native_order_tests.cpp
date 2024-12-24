
#include "Decimal.h"
#include "broker-interface/instruments.hpp"
#include "broker-interface/order.hpp"
#include "ibkr/internal/order_wrapper.hpp"
#include "logging/logging.hpp"
#include "gmock/gmock.h"
#include <boost/signals2/connection.hpp>
#include <gtest/gtest.h>
#include <memory>

using namespace ibkr::internal;

std::shared_ptr<logging::thread_safe_logger_t>
    specLogger(std::make_shared<logging::thread_safe_logger_t>(
        logging::create_channel_logger("specs")));
class MockOrder : public midas::Order {
protected:
  MOCK_METHOD(void, setState, (midas::OrderStatusEnum));

public:
  MockOrder()
      : midas::Order(2, midas::OrderDirection::BUY,
                     midas::InstrumentEnum::MicroNasdaqFutures,
                     midas::ExecutionType::Limit, specLogger) {}
  MOCK_METHOD(void, visit, (midas::OrderVisitor &));
};

struct NativeOrderTest : public testing::Test {
  std::atomic<int> fillCount{0};
  MockOrder mockOrder;
  std::unique_ptr<NativeOrder> orderPtr;
  boost::signals2::scoped_connection fillSignalConnection;



  void SetUp() override {
    fillCount.store(0);
    mockOrder.addFillEventListener(
        [this]([[maybe_unused]] midas::Order &,
               [[maybe_unused]] midas::Order::FillEvent event) {
         if (event.isCompletelyFilled) {
           fillCount.fetch_add(1);
         }
        });
    Order order;
    order.totalQuantity = DecimalFunctions::doubleToDecimal(2);
    orderPtr.reset(new NativeOrder(order, mockOrder, specLogger));
  }
};



TEST_F(NativeOrderTest, SendsFilledSignal) {
  EXPECT_FALSE(orderPtr->inCompletelyFilledState());
  Execution nativeExecution;
  nativeExecution.price = 22108.8;
  nativeExecution.avgPrice = 22108.8;
  nativeExecution.shares = DecimalFunctions::doubleToDecimal(2);
  nativeExecution.cumQty = DecimalFunctions::doubleToDecimal(2);
  nativeExecution.execId = "00019125.675f61e6.01.01";
  nativeExecution.side = "BOT";
  nativeExecution.time = "2011-10-05T14:48:00.000Z";
  nativeExecution.orderId = 13;
  ExecutionEntry parsed(nativeExecution);
  orderPtr->addExecutionEntry(parsed);
  EXPECT_TRUE(orderPtr->inCompletelyFilledState());
  EXPECT_EQ(fillCount.load(), 1);
}
