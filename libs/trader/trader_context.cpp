

#include "trader/trader_context.hpp"
#include "broker-interface/broker.hpp"

#include <atomic>
#include <memory>
#include <thread>
#include <trader/trader.hpp>

using namespace std::chrono_literals;
using namespace midas;
TradingContext::TradingContext(std::atomic<bool> *stopProcessingPtr) {
  broker = createIBKRBroker();
  broker->connect();
  brokerProcessor = std::jthread([this, stopProcessingPtr] {
    while (!stopProcessingPtr->load()) {
      broker->processCycle();
    }
  });
  orderManager = broker->getOrderManager();
}

TraderContext::TraderContext(std::atomic<bool> *stopProcessingPtr,
                             unsigned int numSecondsHistory,
                             TradingContext *context,
                             midas::InstrumentEnum instrument,
                             int entryQuantity)
    : streamPtr(new midas::DataStream(5)),
      trader(midas::trader::momentumExploit(streamPtr, context->orderManager,
                                            instrument, entryQuantity)) {
  // Subscribe to data stream
  auto subscriptionDataHandler =
      [this]([[maybe_unused]] const midas::Subscription &sub, midas::Bar bar) {
        streamPtr->addBars(bar);
      };
  historicalSubscription = std::make_shared<midas::Subscription>(
      instrument,
      midas::HistorySubscriptionStartPoint{
          .unit = midas::SubscriptionDurationUnits::Seconds,
          .quantity = numSecondsHistory},
      false);
  historicalBarConn =
      historicalSubscription->barSignal.connect(subscriptionDataHandler);

  historicalEndCon = historicalSubscription->endSignal.connect(
      [this, context, stopProcessingPtr, subscriptionDataHandler,
       instrument]([[maybe_unused]] const midas::Subscription &sub) {
        realtimeSubscription =
            std::make_shared<midas::Subscription>(instrument, false);
        realtimeBarCon =
            realtimeSubscription->barSignal.connect(subscriptionDataHandler);

        context->broker->addSubscription(realtimeSubscription);
        trader->triggerSourceProcessing();
        streamProcessingThread = std::jthread([stopProcessingPtr, this] {
          while (!stopProcessingPtr->load()) {
            streamPtr->waitForData(100ms);
            trader->decide();
          }
        });
      });
  context->broker->addSubscription(historicalSubscription);
}