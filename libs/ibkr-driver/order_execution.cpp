#include "CommissionReport.h"
#include "Decimal.h"
#include "broker-interface/order.hpp"
#include "ibkr/internal/client.hpp"
#include "ibkr/internal/order_wrapper.hpp"
#include <boost/date_time/posix_time/time_parsers.hpp>

ibkr::internal::ExecutionEntry::ExecutionEntry(const native_execution_t &native)
    : id(native.execId), exchange(native.exchange),
      serverExecutionTime(boost::posix_time::from_iso_string(native.time)),
      direction(native.side == "BUY" ? midas::OrderDirection::BUY
                                     : midas::OrderDirection::SELL),
      quantity(DecimalFunctions::decimalToDouble(native.shares)),
      totalPrice(native.price),
      cumulativeQuantity(DecimalFunctions::decimalToDouble(native.cumQty)),
      averagePrice(native.avgPrice) {}

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