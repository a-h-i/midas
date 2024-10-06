#include "bracketed_order_state.hpp"
#include "broker-interface/order.hpp"
#include "exceptions/order_parameter_error.hpp"
#include "exceptions/order_state_error.hpp"
#include <memory>

midas::BracketedOrder::BracketedOrder(
    unsigned int quantity, OrderDirection direction, InstrumentEnum instrument,
    double entryPrice, double profitPrice, double stopLossPrice,
    std::shared_ptr<logging::thread_safe_logger_t> logger)
    : Order(quantity, direction, instrument, ExecutionType::Limit, logger) {

  if ((direction == OrderDirection::BUY && profitPrice < entryPrice) ||
      (direction == OrderDirection::SELL && profitPrice > entryPrice)) {
    // taking profit below requested  buy or above sale
    CRITICAL_LOG(*logger) << "Requested to attach profit taker to " << direction
                          << " type order with profit taker price of "
                          << profitPrice << " and base order price of "
                          << entryPrice;
    throw OrderParameterError("Profit target below buy or above sale");
  }
  if ((direction == OrderDirection::BUY && stopLossPrice > entryPrice) ||
      (direction == OrderDirection::SELL && stopLossPrice < entryPrice)) {
    // stopping loss above requested buy or below sale
    CRITICAL_LOG(*logger) << "Requested to attach stop loss to " << direction
                          << " type order with stop price of " << stopLossPrice
                          << " and base order price of " << entryPrice;
    throw OrderParameterError("Stop loss target above buy or below sale");
  }
  const OrderDirection exitDirection = ~direction;
  profitTakerOrder =
      std::make_unique<SimpleOrder>(quantity, exitDirection, instrument,
                                    ExecutionType::Limit, logger, profitPrice);
  stopLossOrder =
      std::make_unique<SimpleOrder>(quantity, exitDirection, instrument,
                                    ExecutionType::Stop, logger, stopLossPrice);
  entryOrder =
      std::make_unique<SimpleOrder>(quantity, direction, instrument,
                                    ExecutionType::Limit, logger, entryPrice);
  phasePtr = std::make_unique<internal::BracketUntransmittedState>(this);
  entryOrder->addFillEventListener(
      [this]([[maybe_unused]] Order &entry, FillEvent event) {
        if (event.isCompletelyFilled) {
          handleEntryFilled();
        }
      });
}

void midas::BracketedOrder::handleEntryFilled() {
  phasePtr = std::make_unique<internal::BracketHoldingPositionState>(this);
}

bool midas::BracketedOrder::inModifiableState() const {
  return phasePtr->isModifiable();
}
midas::OrderStatusEnum midas::BracketedOrder::state() const {
  return phasePtr->state();
}

void midas::BracketedOrder::transmit(midas::OrderTransmitter &transmitter) {
  transmitter.transmit(*this);
}

void midas::BracketedOrder::setTransmitted() {
  if (!phasePtr->canTransmit()) {
    CRITICAL_LOG(*logger) << "Tried to set transmitted while order status was "
                          << state();
    throw OrderStateError(
        "Can only enter transmitted state from untransmitted state");
  }
  status = OrderStatusEnum::Accepted;
  phasePtr = std::make_unique<internal::BracketTransmittedState>(this);
}

midas::BracketedOrder::~BracketedOrder() = default;

bool midas::internal::BracketUntransmittedState::canTransmit() { return true; }

bool midas::internal::BracketedOrderState::canTransmit() { return false; }

bool midas::internal::BracketUntransmittedState::isModifiable() const {
  return true;
}

bool midas::internal::BracketedOrderState::isModifiable() const {
  // currently we are keeping it simple and deeming entire bracket
  // unmodifiable, once entry position is transmitted
  return false;
}
midas::OrderStatusEnum
midas::internal::BracketUntransmittedState::state() const {
  return OrderStatusEnum::UnTransmitted;
}

midas::OrderStatusEnum midas::internal::BracketTransmittedState::state() const {
  return OrderStatusEnum::Accepted;
}

midas::OrderStatusEnum
midas::internal::BracketHoldingPositionState::state() const {
  return OrderStatusEnum::WaitingForChildren;
}

midas::OrderStatusEnum midas::internal::BracketTerminatedState::state() const {
  return OrderStatusEnum::Filled;
}
