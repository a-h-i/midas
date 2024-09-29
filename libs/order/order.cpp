#include "broker-interface/order.hpp"
#include "exceptions/order_parameter_error.hpp"
#include <stdexcept>

midas::Order::Order(unsigned int requestedQuantity, OrderDirection direction,
                    InstrumentEnum instrument, ExecutionType type,
                    double targetPrice,
                    std::shared_ptr<logging::thread_safe_logger_t> logger)
    : quantity(requestedQuantity), direction(direction), instrument(instrument),
      execType(type), targetPrice(targetPrice), logger(logger) {}

void midas::Order::Order::attachProfitTaker(double limit) {
  if (!inModifiableState()) {
    CRITICAL_LOG(*logger) << "Attempted to attach profit taker while order is "
                             "in non modifiable state "
                          << state();
    throw std::logic_error("Order not in modifiable state");
  }
  if ((direction == OrderDirection::BUY && limit < targetPrice) ||
      limit > targetPrice) {
    // taking profit below requested  buy or above sale
    CRITICAL_LOG(*logger) << "Requested to attach profit taker to " << direction
                          << " type order with profit taker price of " << limit
                          << " and base order price of " << targetPrice;
    throw OrderParameterError("Profit target below buy or above sale");
  }
  OrderDirection profitTakerDirection = ~direction;
  profitTaker.emplace(quantity, profitTakerDirection, instrument,
                      ExecutionType::Limit, limit, logger);
}

void midas::Order::Order::attachStopLoss(double limit) {
  if (!inModifiableState()) {
    CRITICAL_LOG(*logger) << "Attempted to attach stop loss while order is in "
                             "non modifiable state "
                          << state();
    throw std::logic_error("Order not in modifiable state");
  }
  if ((direction == OrderDirection::BUY && limit > targetPrice) ||
      limit < targetPrice) {
    // stopping loss above requested buy or below sale
    CRITICAL_LOG(*logger) << "Requested to attach stop loss to " << direction
                          << " type order with stop price of " << limit
                          << " and base order price of " << targetPrice;
    throw OrderParameterError("Stop loss target above buy or below sale");
  }
  OrderDirection stopLossDirection = ~direction;
  stopLoss.emplace(quantity, stopLossDirection, instrument, ExecutionType::Stop,
                   limit, logger);
}

bool midas::Order::inModifiableState() const {
  return state() == OrderStatusEnum::UnTransmitted;
}
