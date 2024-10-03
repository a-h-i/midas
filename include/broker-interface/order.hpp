#pragma once
#include "broker-interface/instruments.hpp"
#include "logging/logging.hpp"
#include "observers/observers.hpp"
#include <atomic>
#include <memory>
#include <optional>

namespace midas {

enum class OrderStatusEnum {
  /**
   * Completely filled considered a final state
   */
  Filled,
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
    stream << "filled";
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

enum class ExecutionType { Limit, Stop };

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
struct OrderTransmitter {
  virtual ~OrderTransmitter();
  virtual void transmit(SimpleOrder &) = 0;
  virtual void transmit(BracketedOrder &) = 0;
};

/**
 * Order representation
 */
class Order {

public:
  const ExecutionType execType;

  struct StatusChangeEvent {
    OrderStatusEnum oldStatus, newStatus;
  };
  struct FillEvent {
    unsigned int newFilled;
    bool isCompletelyFilled;
  };

  typedef std::function<void(Order &, StatusChangeEvent)> StatusChangeHandler;
  typedef std::function<void(Order &, FillEvent)> FillEventHandler;
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
  std::optional<double> totalCommissions;
  std::atomic<unsigned int> quantityFilled{0};
  OrderStatusEnum status{OrderStatusEnum::UnTransmitted};
  std::atomic<double> avgFillPrice{0}, lastFillPrice{0};

protected:
  std::shared_ptr<logging::thread_safe_logger_t> logger;
  EventSubject<StatusChangeHandler> statusObservers;
  EventSubject<FillEventHandler> fillHandlers;

public:
  /**
   * \param requestedQuantity
   */
  Order(unsigned int requestedQuantity, OrderDirection direction,
        InstrumentEnum instrument, ExecutionType execType,
        std::shared_ptr<logging::thread_safe_logger_t> logger);
  virtual ~Order();
  /**
   * visitor pattern
   */
  virtual void transmit(OrderTransmitter &) = 0;

  virtual bool inModifiableState() const;
  virtual OrderStatusEnum state() const;

  virtual decltype(statusObservers)::ListenerIdType
  addStatusChangeListener(StatusChangeHandler handler) {
    return statusObservers.add_listener(handler);
  }
  virtual void
  removeStatusChangeListener(decltype(statusObservers)::ListenerIdType id) {
    statusObservers.remove_listener(id);
  }

  virtual decltype(fillHandlers)::ListenerIdType
  addFillEventListener(FillEventHandler handler) {
    return fillHandlers.add_listener(handler);
  }
  virtual void
  removeFillEventListener(decltype(fillHandlers)::ListenerIdType id) {
    fillHandlers.remove_listener(id);
  }
};

/**
 * Orders that work with limit target prices
 */
class SimpleOrder : public Order {

private:
  const double targetPrice;

public:
  SimpleOrder(unsigned int requestedQuantity, OrderDirection direction,
              InstrumentEnum instrument, ExecutionType type,
              std::shared_ptr<logging::thread_safe_logger_t> logger,
              double targetPrice);
  virtual void transmit(OrderTransmitter &) override;
};
namespace internal {
class BracketedOrderState;
} // namespace internal
class BracketedOrder : public Order {
  friend class internal::BracketedOrderState;

private:
  std::unique_ptr<Order> entryOrder, stopLossOrder, profitTakerOrder;
  std::unique_ptr<internal::BracketedOrderState> phasePtr;

public:
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
  virtual void transmit(OrderTransmitter &) override;
};

/**
 * Manages and executes orders
 */
class OrderManager {

public:
  virtual ~OrderManager() = default;
  virtual void transmit(std::shared_ptr<Order>) = 0;
  virtual bool hasActiveOrders() const = 0;

};
} // namespace midas