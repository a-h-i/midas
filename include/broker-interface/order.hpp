#pragma once
#include "broker-interface/instruments.hpp"
#include "logging/logging.hpp"
#include "observers/observers.hpp"
#include <memory>
#include <optional>

namespace midas {

enum class OrderStatusEnum {
  /**
   * Completely filled considered a final state
   */
  Filled,
  /**
   * Filled But waiting for children
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

/**
 * Abstract order representation
 */
class Order {
public:
  enum class EventSource { ParentOrder, ProfitTaker, StopLoss };

  struct BaseEvent {
    EventSource source;
  };

  struct StatusChangeEvent : BaseEvent {
    OrderStatusEnum oldStatus, newStatus;
  };
  struct FillEvent : BaseEvent {
    unsigned int newFilled;
    bool isCompletelyFilled;
  };

  typedef std::function<void(Order &, StatusChangeEvent)> StatusChangeHandler;
  typedef std::function<void(Order &, FillEvent)> FillEventHandler;

private:
  /**
   * Total quantity requested in order
   */
  const unsigned int quantity;
  unsigned int quantityFilled = 0;
  std::optional<double> avgFillPrice, lastFillPrice;
  const OrderDirection direction;
  const InstrumentEnum instrument;
  OrderStatusEnum status{OrderStatusEnum::UnTransmitted};
  std::unique_ptr<Order> profitTaker, stopLoss;
  ExecutionType execType;
  double targetPrice;
  /**
   * Only available after full execution
   */
  std::optional<double> totalCommissions;
  std::shared_ptr<logging::thread_safe_logger_t> logger;
  EventSubject<StatusChangeHandler> statusObservers;
  EventSubject<FillEventHandler> fillHandlers;

public:
  Order(unsigned int requestedQuantity, OrderDirection direction,
        InstrumentEnum instrument, ExecutionType type, double targetPrice,
        std::shared_ptr<logging::thread_safe_logger_t> logger);
  virtual ~Order() = default;

  virtual void cancel();
  void attachProfitTaker(double limit);
  void attachStopLoss(double limit);
  bool inModifiableState() const;

  inline OrderStatusEnum state() const { return status; }

  auto addStatusChangeListener(StatusChangeHandler handler) {
    return statusObservers.add_listener(handler);
  }
  void
  removeStatusChangeListener(decltype(statusObservers)::ListenerIdType id) {
    statusObservers.remove_listener(id);
  }

  auto addFillEventListener(FillEventHandler handler) {
    return fillHandlers.add_listener(handler);
  }
  void removeFillEventListener(decltype(fillHandlers)::ListenerIdType id) {
    fillHandlers.remove_listener(id);
  }
};

/**
 * Manages and executes orders
 */
class OrderManager {

public:
  virtual ~OrderManager() = default;
  virtual void transmit(std::shared_ptr<Order>) = 0;
};
} // namespace midas