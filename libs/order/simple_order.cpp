#include "broker-interface/order.hpp"

midas::SimpleOrder::SimpleOrder(
    unsigned int requestedQuantity, OrderDirection direction,
    InstrumentEnum instrument, ExecutionType type,
    std::shared_ptr<logging::thread_safe_logger_t> logger, double targetPrice)
    : Order(requestedQuantity, direction, instrument, type, logger),
      targetPrice(targetPrice) {}

void midas::SimpleOrder::transmit(midas::OrderTransmitter &transmitter) {
  transmitter.transmit(*this);
}