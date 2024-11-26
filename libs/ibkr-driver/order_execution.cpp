#include "CommissionReport.h"
#include "Decimal.h"
#include "broker-interface/order.hpp"
#include "ibkr/internal/client.hpp"
#include "ibkr/internal/order_wrapper.hpp"
#include <boost/date_time/posix_time/time_parsers.hpp>
#include <memory>
#include <stdexcept>
#include <string>

ibkr::internal::ExecutionEntry::ExecutionEntry(const native_execution_t &native)
    : id(native.execId), exchange(native.exchange),
      serverExecutionTime(boost::posix_time::from_iso_string(native.time)),
      direction(native.side == "BUY" ? midas::OrderDirection::BUY
                                     : midas::OrderDirection::SELL),
      quantity(DecimalFunctions::decimalToDouble(native.shares)),
      totalPrice(native.price),
      cumulativeQuantity(DecimalFunctions::decimalToDouble(native.cumQty)),
      averagePrice(native.avgPrice) {}

std::string ibkr::internal::ExecutionEntry::getBaseId() const {
  const auto pos = id.find(".");
  if (pos == id.npos) {
    return id;
  } else {
    return id.substr(0, pos);
  }
}

bool ibkr::internal::ExecutionEntry::corrects(
    const ExecutionEntry &other) const {
  const auto posSelf = id.find(".");
  const auto posOther = other.id.find(".");
  if (posSelf == id.npos || posOther == other.id.npos) {
    return false;
  }
  const auto indexSelf = std::stoi(id.substr(posSelf));
  const auto indexOther = std::stoi(other.id.substr(posOther));
  return getBaseId() == other.getBaseId() && indexSelf > indexOther;
}

ibkr::internal::CommissionEntry::CommissionEntry(const CommissionReport &native)
    : executionId(native.execId), currency(native.currency),
      commission(native.commission), realizedPNL(native.realizedPNL) {}

void ibkr::internal::Client::handleExecution(
    const native_execution_t &execution) {

  activeOrders.visit(execution.orderId, [&execution](const auto &pair) {
    pair.second->addExecutionEntry(ExecutionEntry(execution));
  });
}

void ibkr::internal::Client::handleCommissionReport(
    const CommissionReport &commission) {
  // reports can arrive out of order relative to executions.
  // We will store them until we get an order completely filled event.

  unhandledCommissions.insert({commission.execId, commission});
}

void ibkr::internal::Client::handleOrderCompletelyFilledEvent(OrderId orderId) {

  std::shared_ptr<NativeOrder> order;
  activeOrders.visit(orderId, [&order](auto &pair) { order = pair.second; });
  if (!order) {
    throw std::runtime_error("Could not find active order with id " +
                             std::to_string(orderId));
  }
  // now we must get all executions
  unhandledCommissions.erase_if([&order](const auto &pair) {
    return order->addCommissionEntry(pair.second);
  });
  order->cleanCorrectedExecutions();
  activeOrders.erase(orderId);
  order->setCompletelyFilled();
}