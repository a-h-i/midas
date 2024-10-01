#include "broker-interface/order.hpp"

midas::Order::Order(unsigned int requestedQuantity, OrderDirection direction,
                    InstrumentEnum instrument, ExecutionType execType,
                    std::shared_ptr<logging::thread_safe_logger_t> logger)
    : execType(execType), requestedQuantity(requestedQuantity),
      direction(direction), instrument(instrument), logger(logger) {}

bool midas::Order::inModifiableState() const {
  return state() == OrderStatusEnum::UnTransmitted;
}

midas::OrderStatusEnum midas::Order::state() const { return status; }