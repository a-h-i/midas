#pragma once

#include "broker-interface/broker.hpp"
#include "broker-interface/instruments.hpp"
#include "broker-interface/order.hpp"
#include "data/data_stream.hpp"
#include "trader/base_trader.hpp"
#include <atomic>
#include <boost/signals2/connection.hpp>
#include <memory>
#include <thread>

struct TradingContext {
  std::shared_ptr<midas::Broker> broker;
  std::shared_ptr<midas::OrderManager> orderManager;
  std::jthread brokerProcessor;

  TradingContext(std::atomic<bool> *stopProcessing);
};

struct TraderContext {
  std::shared_ptr<midas::DataStream> streamPtr;
  std::unique_ptr<midas::trader::Trader> trader;
  std::jthread streamProcessingThread;
  boost::signals2::scoped_connection historicalBarConn, historicalEndCon,
      realtimeBarCon;
  std::shared_ptr<midas::Subscription> historicalSubscription,
      realtimeSubscription;
  TraderContext(std::atomic<bool> *, unsigned int numSecondsHistory,
                TradingContext *, midas::InstrumentEnum);
};