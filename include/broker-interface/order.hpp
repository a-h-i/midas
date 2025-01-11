#pragma once
#include "broker-interface/instruments.hpp"
#include "logging/logging.hpp"
#include "position_tracker.hpp"
#include <atomic>
#include <boost/signals2.hpp>
#include <memory>
#include <mutex>

namespace midas {

enum class OrderStatusEnum {
  /**
   * Completely filled considered a final state
   */
  Filled,
  /**
   * Parent executed, waiting for children
   */
  WaitingForChildren,
  /**
   * Considered a final state
   */
  Cancelled,
  /**
   * Transmitted but not accepted by destination
   */
  PendingAccept,
  /**
   * Cancel request transmitted but not confirmed by destination
   */
  PendingCancel,
  /**
   * Submitted and accepted by destination
   */
  Accepted,
  /**
   * Held while broker is attempting to locate shares to short sell
   */
  ShortLocatingHold,
  /**
   * Not transmitted by application.
   * Start state
   */
  UnTransmitted,
};
template <typename CharT, typename TraitsT>
std::basic_ostream<CharT, TraitsT> &
operator<<(std::basic_ostream<CharT, TraitsT> &stream, OrderStatusEnum status) {
  switch (status) {
  case OrderStatusEnum::Filled:
    stream << "Filled";
    break;
  case OrderStatusEnum::WaitingForChildren:
    stream << "WaitingForChildren";
    break;
  case OrderStatusEnum::Cancelled:
    stream << "Cancelled";
    break;
  case OrderStatusEnum::PendingAccept:
    stream << "PendingAccept";
    break;
  case OrderStatusEnum::PendingCancel:
    stream << "PendingCancel";
    break;
  case OrderStatusEnum::Accepted:
    stream << "Accepted";
    break;
  case OrderStatusEnum::ShortLocatingHold:
    stream << "ShortLocatingHold";
    break;
  case OrderStatusEnum::UnTransmitted:
    stream << "UnTransmitted";
  }
  return stream;
};

enum class OrderDirection { BUY, SELL };
template <typename CharT, typename TraitsT>
std::basic_ostream<CharT, TraitsT> &
operator<<(std::basic_ostream<CharT, TraitsT> &stream,
           OrderDirection direction) {
  switch (direction) {
  case OrderDirection::BUY:
    stream << "BUY";
    break;
  case OrderDirection::SELL:
    stream << "SELL";
    break;
  }
  return stream;
};

OrderDirection operator~(OrderDirection original);

enum class ExecutionType { Limit, Stop, MKT };

template <typename CharT, typename TraitsT>
std::basic_ostream<CharT, TraitsT> &
operator<<(std::basic_ostream<CharT, TraitsT> &stream,
           ExecutionType execution) {
  switch (execution) {
  case ExecutionType::Limit:
    stream << "LMT";
    break;
  case ExecutionType::Stop:
    stream << "STP";
    break;
  case ExecutionType::MKT:
    stream << "MKT";
    break;
  }
  return stream;
};

class SimpleOrder;
class BracketedOrder;

/**
 * \brief An interface for implementing a visitor design pattern.
 * \details  Why do we need it ? an order transmitter is responsible for
 * serializing orders to different platforms and backends. These could be
 * simulations or different brokers. Each backend has different order semantics,
 * additionally different order subtypes have different fields some relative
 * orders have targets in percentage of parent, or basis points, or absolute
 * price, etc. We want to avoid a situation where classes members that have
 * different meanings (i.e one target that can either be in relative or absolute
 * units) We also don't want multiple members on all subclasses with only a
 * subset in use and valid. This allows us to do double dispatch without
 * resorting to dynamic casts and if statements as that is very ugly in OO
 * design. There is of course a degree of opinion here.
 */
struct OrderVisitor {
  virtual ~OrderVisitor() = default;
  virtual void visit(SimpleOrder &) = 0;
  virtual void visit(BracketedOrder &) = 0;
};

/**
 * Order representation
 */
class Order {

public:
  /**
   * Midas order ids
   * note that brokers may have different ids and thus these ids must be
   * translated between by implementations
   */
  const unsigned int id;
  const ExecutionType execType;

  struct StatusChangeEvent {
    OrderStatusEnum oldStatus, newStatus;
  };
  struct FillEvent {
    unsigned int newFilled;
    double price;
    double commission;
    bool isCompletelyFilled;
  };

  typedef boost::signals2::signal<void(Order &, StatusChangeEvent)>
      status_change_signal_t;
  typedef boost::signals2::signal<void(Order &, FillEvent)> fill_event_signal_t;
  /**
   * Total quantity requested in order
   */
  const unsigned int requestedQuantity;
  const OrderDirection direction;
  const InstrumentEnum instrument;

private:
  /**
   * Only available after full execution
   */
  std::atomic<double> totalCommissions{0};
  std::atomic<unsigned int> quantityFilled{0};
  std::atomic<double> avgFillPrice{0};
  std::atomic<OrderStatusEnum> status{OrderStatusEnum::UnTransmitted};

protected:
  std::shared_ptr<logging::thread_safe_logger_t> logger;
  fill_event_signal_t fillEventSignal;
  status_change_signal_t statusChangeSignal;

  virtual void setState(OrderStatusEnum newState);
  virtual void setFilledNoStatusUpdate(double avgFillPrice,
                                       double totalCommissions,
                                       unsigned int filledQuantity);

public:
  /**
   * \param requestedQuantity
   */
  Order(unsigned int requestedQuantity, OrderDirection direction,
        InstrumentEnum instrument, ExecutionType execType,
        std::shared_ptr<logging::thread_safe_logger_t> logger);
  virtual ~Order() = default;
  /**
   * visitor pattern
   */
  virtual void visit(OrderVisitor &) = 0;
  virtual void setTransmitted();
  virtual void setCancelled();
  virtual void setShortLocatingHold();
  virtual void setFilled(double avgFillPrice, double totalCommissions,
                         unsigned int filledQuantity);
  virtual bool inModifiableState() const;
  inline bool isDone() const {
    auto current = state();
    return current == OrderStatusEnum::Filled ||
           current == OrderStatusEnum::Cancelled;
  }
  virtual OrderStatusEnum state() const;

  virtual boost::signals2::connection
  addStatusChangeListener(const status_change_signal_t::slot_type &handler) {
    return statusChangeSignal.connect(handler);
  }

  virtual boost::signals2::connection
  addFillEventListener(const fill_event_signal_t::slot_type &handler) {
    return fillEventSignal.connect(handler);
  }

  /**
   * Only valid if order in filled state.
   */
  virtual double getAvgFillPrice();
  virtual unsigned int getFilledQuantity();

  bool operator==(const Order &other) const;
};

/**
 * Orders that work with limit target prices
 */
class SimpleOrder : public Order {

public:
  double targetPrice;
  SimpleOrder(unsigned int requestedQuantity, OrderDirection direction,
              InstrumentEnum instrument, ExecutionType type,
              std::shared_ptr<logging::thread_safe_logger_t> logger,
              double targetPrice);
  virtual void visit(OrderVisitor &) override;
};

namespace internal {
class BracketedOrderState;
} // namespace internal

class BracketedOrder : public Order {

private:
  std::unique_ptr<SimpleOrder> entryOrder, stopLossOrder, profitTakerOrder;
  std::unique_ptr<internal::BracketedOrderState> phasePtr;
  std::recursive_mutex phaseMutex;
  boost::signals2::connection entryFillConnection, stopLossFillConnection,
      profitTakerFillConnection;
  void handleEntryFilled(FillEvent event);
  void handleStopLossFilled(FillEvent event);
  void handleProfitTakerFilled(FillEvent event);

public:
  friend class midas::internal::BracketedOrderState;
  /**
   * Bracket orders have both directions attached as part of stop loss and
   * profit taker. Direction param indicates entry direction of order. i.e a
   * short position that is bracketed or a long position that is bracketed.
   *
   * \note currently entry order type is always limit
   */
  BracketedOrder(unsigned int quantity, OrderDirection direction,
                 InstrumentEnum instrument, double entryPrice,
                 double profitPrice, double stopLossPrice,
                 std::shared_ptr<logging::thread_safe_logger_t> logger);
  ~BracketedOrder();
  virtual bool inModifiableState() const override;
  virtual OrderStatusEnum state() const override;
  virtual void visit(OrderVisitor &) override;
  virtual void setTransmitted() override;
  virtual void setCancelled() override;
  inline SimpleOrder &getEntryOrder() { return *entryOrder; }
  inline SimpleOrder &getStopOrder() { return *stopLossOrder; }
  inline SimpleOrder &getProfitTakerOrder() { return *profitTakerOrder; }
  virtual void setFilled(double avgFillPrice, double totalCommissions,
                         unsigned int filledQuantity) override;
};

/**
 * Manages and executes orders
 */
class OrderManager {
public:

  PositionTracker positionTracker;
protected:
  std::shared_ptr<logging::thread_safe_logger_t> logger;
  virtual void handleRealized(InstrumentEnum instrument, int quantity, double pricePerUnit);
public:
  OrderManager(std::shared_ptr<logging::thread_safe_logger_t> &logger)
      : logger(logger) {}
  virtual ~OrderManager() = default;
  virtual void transmit(std::shared_ptr<Order>) = 0;
  virtual bool hasActiveOrders() = 0;
  virtual std::list<Order *> getFilledOrders() = 0;



};
} // namespace midas