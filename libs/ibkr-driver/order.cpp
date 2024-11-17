#include "Order.h"
#include "CommissionReport.h"
#include "Execution.h"
#include "OrderState.h"
#include "ibkr/internal/client.hpp"
#include "logging/logging.hpp"

void ibkr::internal::Client::orderStatus(
    OrderId orderId, const std::string &status, Decimal filled,
    Decimal remaining, double avgFillPrice, [[maybe_unused]] int permId,
    [[maybe_unused]] int parentId, [[maybe_unused]] double lastFillPrice,
    [[maybe_unused]] int clientId, [[maybe_unused]] const std::string &whyHeld,
    [[maybe_unused]] double mktCapPrice) {

  DEBUG_LOG(logger) << "ORDER STATUS: OrderId " << orderId
                    << " status: " << status << " filled " << filled
                    << " remaining " << remaining << " avg fill price "
                    << avgFillPrice;
}

void ibkr::internal::Client::openOrder(OrderId orderId,
                                       const Contract &contract,
                                       const Order &order,
                                       const OrderState &state) {
  DEBUG_LOG(logger) << "OPEN ORDER: OrderID" << orderId << " CONTRACT "
                    << contract.symbol << " action" << order.action
                    << " STATE: " << state.status;
}
void ibkr::internal::Client::openOrderEnd() {
  DEBUG_LOG(logger) << "OPEN ORDER END";
}

void ibkr::internal::Client::winError(const std::string &str,
                                      [[maybe_unused]] int lastError) {
  CRITICAL_LOG(logger) << "RECEIVED WINERROR " << str;
}

void ibkr::internal::Client::connectionClosed() {
  ERROR_LOG(logger) << "Connection closed called";
}

void ibkr::internal::Client::execDetails(int reqId, const Contract &contract,
                                         const Execution &execution) {
  DEBUG_LOG(logger) << "ExecDetails " << reqId
                    << " contract: " << contract.symbol << " execution "
                    << execution.side << " shared " << execution.shares;
}

void ibkr::internal::Client::execDetailsEnd(int reqId) {
  DEBUG_LOG(logger) << "ExecDetailsEnd " << reqId;
}

void ibkr::internal::Client::commissionReport(
    const CommissionReport &commissionReport) {
  DEBUG_LOG(logger) << "Received commission report" << commissionReport.execId
                    << " commission " << commissionReport.commission << " pnl "
                    << commissionReport.realizedPNL;
}

void ibkr::internal::Client::completedOrdersEnd() {
  DEBUG_LOG(logger) << "Completed order end";
}

void ibkr::internal::Client::processPendingOrders() {
  std::scoped_lock lock(ordersMutex);
  for (auto &order : pendingOrders) {
    activeOrders[order->nativeOrder.orderId] = order;
    order->setTransmitted();
    connectionState.clientSocket->placeOrder(
        order->nativeOrder.orderId, order->ibkrContract, order->nativeOrder);
  }
  // we have processed all pending orders
  pendingOrders.clear();
}