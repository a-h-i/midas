#pragma once

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

enum class OrderDirection { BUY, SELL };

/**
 * Abstract order representation
 */
class Order {
  /**
   * Total quantity requested in order
   */
  const unsigned int quantity;
  unsigned int quantityFilled;
  double avgFillPrice, lastFillPrice;
  const OrderDirection direction;

public:
  Order(unsigned int requestedQuantity, OrderDirection direction);
  virtual ~Order();
  /**
   * Unique order ids
   */
  virtual int orderId() const = 0;
};


/**
 * Manages and executes orders
 */
class OrderManager {
  
  public:
    virtual ~OrderManager() = default;
};
} // namespace midas