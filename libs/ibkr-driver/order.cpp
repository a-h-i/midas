#include "Order.h"
#include "CommissionReport.h"
#include "Decimal.h"
#include "Execution.h"
#include "OrderState.h"
#include "ibkr/internal/client.hpp"
#include "ibkr/internal/order_wrapper.hpp"
#include "logging/logging.hpp"
#include <memory>
#include <mutex>

void ibkr::internal::Client::orderStatus(
    OrderId orderId, const std::string &status, Decimal filled,
    Decimal remaining, double avgFillPrice, int permId, int parentId,
    double lastFillPrice, int clientId, const std::string &whyHeld,
    double mktCapPrice) {

  INFO_LOG(logger) << "ORDER STATUS: OrderId " << orderId
                   << " status: " << status << " filled "
                   << DecimalFunctions::decimalToString(filled) << " remaining "
                   << DecimalFunctions::decimalToString(remaining)
                   << " avg fill price " << avgFillPrice << " permId " << permId
                   << " parentId " << parentId << " lastFillPrice "
                   << lastFillPrice << " clientId " << clientId << " whyHeld "
                   << whyHeld << " mktCapPrice " << mktCapPrice;
  std::scoped_lock commandsLock(commandsMutex);
  pendingCommands.push_back([this, orderId, status]() {
    if (!activeOrders.contains(orderId)) {
      // sometimes events are duplicated
      WARNING_LOG(logger) << "Can not find active order entry for order "
                              "status event with order id: "
                           << orderId  << " status " << status;
      return;
    }
    std::shared_ptr<NativeOrder> order;

    activeOrders.visit(orderId,
                       [&order](const auto &pair) { order = pair.second; });
    // https://www.interactivebrokers.com/campus/ibkr-api-page/twsapi-doc/#order-status
    if (status == "PendingSubmit") {
      // ignore
    } else if (status == "PendingCancel") {
      //  ignore
    } else if (status == "PreSubmitted") {
      // ignore
    } else if (status == "Submitted") {
      order->setTransmitted();
    } else if (status == "ApiCancelled") {
      order->setCancelled();
      pendingCommands.push_back([this, orderId]() {
        activeOrders.erase(orderId);
      });
    } else if (status == "Cancelled") {
      order->setCancelled();
      pendingCommands.push_back([this, orderId]() {
        activeOrders.erase(orderId);
      });
    } else if (status == "Filled") {
      // indicates that the order has been completely filled. Market orders
      // executions will not always trigger a Filled status.
      pendingCommands.push_back([this, orderId]() {
        handleOrderCompletelyFilledEvent(orderId); // erases order
      });
    } else if (status == "Inactive") {
      // ignore
    } else {
      CRITICAL_LOG(logger) << "Received unknown order status event: " << status
                           << " orderId: " << orderId;
    }
  });
}

void ibkr::internal::Client::openOrder(OrderId orderId,
                                       const Contract &contract,
                                       const Order &order,
                                       const OrderState &state) {
  INFO_LOG(logger) << "OPEN ORDER: OrderID " << orderId << " CONTRACT "
                   << contract.symbol << " action " << order.action
                   << " STATE: " << state.status;
}
void ibkr::internal::Client::openOrderEnd() {
  INFO_LOG(logger) << "OPEN ORDER END";
}

void ibkr::internal::Client::winError(const std::string &str,
                                      [[maybe_unused]] int lastError) {
  CRITICAL_LOG(logger) << "RECEIVED WINERROR " << str;
}
void ibkr::internal::Client::connectionClosed() {
  ERROR_LOG(logger) << "Connection closed called";
}

void ibkr::internal::Client::execDetails(
    [[maybe_unused]] int reqId, [[maybe_unused]] const Contract &contract,
    const Execution &execution) {
  INFO_LOG(logger) << "received execution details";
  std::scoped_lock commandsLock(commandsMutex);
  pendingCommands.push_back([this, execution] { handleExecution(execution); });
}

void ibkr::internal::Client::execDetailsEnd(int reqId) {
  INFO_LOG(logger) << "ExecDetailsEnd " << reqId;
}

void ibkr::internal::Client::commissionReport(
    const CommissionReport &commissionReport) {
  INFO_LOG(logger) << "received commission report";
  std::scoped_lock commandsLock(commandsMutex);
  pendingCommands.push_back(
      [this, commissionReport] { handleCommissionReport(commissionReport); });
}

void ibkr::internal::Client::completedOrdersEnd() {
  INFO_LOG(logger) << "Completed order end";
}

void ibkr::internal::Client::processPendingOrders() {
  std::scoped_lock lock(ordersMutex);
  for (auto &order : pendingOrders) {
    activeOrders.insert({order->nativeOrder.orderId, order});
    INFO_LOG(logger) << "IBKR driver placing order: "
                     << order->ibkrContract.symbol << " "
                     << order->nativeOrder.orderId
                     << " - sec type: " << order->ibkrContract.secType;
    connectionState.clientSocket->placeOrder(
        order->nativeOrder.orderId, order->ibkrContract, order->nativeOrder);
  }
  // we have processed all pending orders
  pendingOrders.clear();
}