#pragma once
#include "trader_data.hpp"
#include <boost/signals2.hpp>
#include "data/data_stream.hpp"
#include "broker-interface/order_summary.hpp"
#include "broker-interface/order.hpp"
#include "broker-interface/instruments.hpp"
namespace midas::trader {

/**
 * An abstract base class for traders. (trading algorithms)
 * We have decided not to include a member in the base class specifying the
 * associated financial instrument as in the future we may opt for algorithms
 * that trade multiple instruments at the same time.
 *
 *
 */
class Trader {
public:
  typedef boost::signals2::signal<void(TradeSummary)> signal_t;

private:
  std::deque<std::shared_ptr<Order>> currentOrders, executedOrders;
  signal_t summarySignal;
  OrderSummaryTracker summary;
  std::recursive_mutex orderStateMutex;

  void handleOrderStatusChangeEvent(Order &order,
                                    Order::StatusChangeEvent event);

protected:
  TraderData data;
  std::shared_ptr<midas::OrderManager> orderManager;
  std::shared_ptr<logging::thread_safe_logger_t> logger;
  Trader(std::size_t lookBackSize, std::size_t candleSizeSeconds,
         std::shared_ptr<DataStream> source,
         std::shared_ptr<midas::OrderManager> orderManager,
         std::shared_ptr<logging::thread_safe_logger_t> logger)
      : data(lookBackSize, candleSizeSeconds, source),
        orderManager(orderManager), logger(logger) {
    // Perform initial processing, for any data already loaded
    data.processSource();
  }

  void enterBracket(InstrumentEnum instrument, unsigned int quantity,
                    OrderDirection direction, double entryPrice,
                    double stopLossPrice, double profitPrice);

public:
  virtual ~Trader() = default;
  virtual void decide() = 0;
  bool hasOpenPosition();
  boost::signals2::connection
  connectSlot(const signal_t::slot_type &subscriber);
};
} // namespace midas::trader