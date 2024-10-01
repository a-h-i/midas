#include "broker-interface/order.hpp"
#include "exceptions/order_parameter_error.hpp"

midas::BracketedOrder::BracketedOrder(
    unsigned int quantity, OrderDirection direction, InstrumentEnum instrument,
    double entryPrice, double profitPrice, double stopLossPrice,
    std::shared_ptr<logging::thread_safe_logger_t> logger)
    : Order(requestedQuantity, direction, instrument, ExecutionType::Limit,
            logger) {

  if ((direction == OrderDirection::BUY && profitPrice < entryPrice) ||
      profitPrice > entryPrice) {
    // taking profit below requested  buy or above sale
    CRITICAL_LOG(*logger) << "Requested to attach profit taker to " << direction
                          << " type order with profit taker price of "
                          << profitPrice << " and base order price of "
                          << entryPrice;
    throw OrderParameterError("Profit target below buy or above sale");
  }
  if ((direction == OrderDirection::BUY && stopLossPrice > entryPrice) ||
      stopLossPrice < entryPrice) {
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
}
