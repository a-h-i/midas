#pragma once
#include "broker-interface/instruments.hpp"
#include "logging/logging.hpp"
#include <memory>
#include <optional>

namespace midas {

enum class OrderStatusEnum {
  /**
   * Completely filled
   */
  Filled,
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
   * Not transmitted by application
   */
  UnTransmitted,
};
template <typename CharT, typename TraitsT>
std::basic_ostream<CharT, TraitsT> &
operator<<(std::basic_ostream<CharT, TraitsT> &stream, OrderStatusEnum status) {
  switch (direction) {
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

OrderDirection operator~(OrderDirection original) {
  if (original == OrderDirection::BUY) {
    return OrderDirection::SELL;
  } else {
    return OrderDirection::BUY;
  }
}

enum class ExecutionType { Limit, Stop };

/**
 * Abstract order representation
 */
class Order {
  /**
   * Total quantity requested in order
   */
  const unsigned int quantity;
  unsigned int quantityFilled = 0;
  std::optional<double> avgFillPrice, lastFillPrice;
  const OrderDirection direction;
  const InstrumentEnum instrument;
  OrderStatusEnum status{OrderStatusEnum::UnTransmitted};
  std::optional<Order> profitTaker, stopLoss;
  ExecutionType execType;
  double targetPrice;
  /**
   * Only available after full execution
   */
  std::optional<double> totalCommissions;
  std::shared_ptr<logging::thread_safe_logger_t> logger;

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
};

/**
 * Manages and executes orders
 */
class OrderManager {

public:
  virtual ~OrderManager() = default;
};
} // namespace midas