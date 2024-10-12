#include "broker-interface/order.hpp"
#include "exceptions/order_state_error.hpp"
#include "logging/logging.hpp"
#include <atomic>

static std::atomic_uint ORDER_ID_COUNTER{0};

midas::Order::Order(unsigned int requestedQuantity, OrderDirection direction,
                    InstrumentEnum instrument, ExecutionType execType,
                    std::shared_ptr<logging::thread_safe_logger_t> logger)
    : id(++ORDER_ID_COUNTER), execType(execType),
      requestedQuantity(requestedQuantity), direction(direction),
      instrument(instrument), logger(logger) {}

bool midas::Order::inModifiableState() const {
  return state() == OrderStatusEnum::UnTransmitted;
}

midas::OrderStatusEnum midas::Order::state() const { return status.load(); }

void midas::Order::setTransmitted() {
  if (status != OrderStatusEnum::UnTransmitted) {
    CRITICAL_LOG(*logger) << "Tried to set transmitted while order status was "
                          << state();
    throw OrderStateError(
        "Can only enter transmitted state from untransmitted state");
  }
  status = OrderStatusEnum::Accepted;
}

void midas::Order::setCancelled() { status = OrderStatusEnum::Cancelled; }

void midas::Order::setShortLocatingHold() {
  status = OrderStatusEnum::ShortLocatingHold;
}

void midas::Order::setFilled(double avgFillPrice, double totalCommissions,
                             unsigned int filledQuantity) {
  this->avgFillPrice.store(avgFillPrice, std::memory_order::release);
  this->totalCommissions.store(totalCommissions, std::memory_order::release);
  this->quantityFilled.store(filledQuantity, std::memory_order::release);
  FillEvent event{
      .newFilled = filledQuantity,
      .isCompletelyFilled = filledQuantity == requestedQuantity,
  };
  if (event.isCompletelyFilled) {
    setState(OrderStatusEnum::Filled);
  }
  fillHandlers.notify(*this, event);
}

void midas::Order::setState(OrderStatusEnum newState) {
  auto oldState = state();
  status.store(newState);
  StatusChangeEvent event{
      .oldStatus = oldState,
      .newStatus = newState,
  };
  statusObservers.notify(*this, event);
}

double midas::Order::getAvgFillPrice() { return avgFillPrice.load(); }

unsigned int midas::Order::getFilledQuantity() { return quantityFilled.load(); }

bool midas::Order::operator==(const Order &other) const { return id == other.id; }