#pragma once
#include "broker-interface/instruments.hpp"
#include "broker-interface/order.hpp"
#include "broker-interface/order_summary.hpp"
#include "data/data_stream.hpp"
#include "logging/logging.hpp"
#include "trader_data.hpp"
#include <boost/signals2.hpp>
#include <boost/signals2/connection.hpp>
#include <boost/signals2/variadic_signal.hpp>
#include <memory>
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
  typedef boost::signals2::signal<void(TradeSummary)> trade_summary_signal_t;
  typedef std::pair<std::string, bool> decision_param_t;
  typedef std::vector<decision_param_t> decision_params_t;
  typedef boost::signals2::signal<void(decision_params_t &)>
      decision_params_signal_t;

private:
  std::deque<std::shared_ptr<Order>> currentOrders, executedOrders;
  trade_summary_signal_t summarySignal;
  OrderSummaryTracker summary;
  std::recursive_mutex orderStateMutex;
  bool isPaused{false};

protected:
  TraderData data;
  std::shared_ptr<midas::OrderManager> orderManager;
  std::shared_ptr<logging::thread_safe_logger_t> logger;
  decision_params_signal_t decisionParamsSignal;
  Trader(std::size_t lookBackSize, std::size_t candleSizeSeconds,
         std::shared_ptr<DataStream> source,
         std::shared_ptr<midas::OrderManager> orderManager,
         std::shared_ptr<logging::thread_safe_logger_t> logger)
      : data(lookBackSize, candleSizeSeconds, source),
        orderManager(orderManager), logger(logger) {
    // Perform initial processing, for any data already loaded
    data.processSource();
  }

  void handleOrderStatusChangeEvent(Order &order,
                                    Order::StatusChangeEvent event);

  void enterBracket(InstrumentEnum instrument, unsigned int quantity,
                    OrderDirection direction, double entryPrice,
                    double stopLossPrice, double profitPrice);
  void enterBracket(std::shared_ptr<BracketedOrder> order);

public:
  virtual ~Trader() = default;
  virtual void decide() = 0;
  virtual std::string traderName() const = 0;
  bool hasOpenPosition();
  boost::signals2::connection
  connectSummarySlot(const trade_summary_signal_t::slot_type &subscriber);
  boost::signals2::connection connectDecisionParamsSlot(
      const decision_params_signal_t::slot_type &subscriber);
  inline bool togglePause() { return isPaused = !isPaused; }
  inline bool paused() { return isPaused; }
};
} // namespace midas::trader