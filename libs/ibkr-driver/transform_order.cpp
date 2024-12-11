#include "CommonDefs.h"
#include "Decimal.h"
#include "broker-interface/order.hpp"
#include "ibkr/internal/ibkr_order_manager.hpp"
#include "ibkr/internal/order_wrapper.hpp"
#include <algorithm>
#include <iterator>
#include <memory>

inline std::string orderType(midas::ExecutionType execution) {
  switch (execution) {

  case midas::ExecutionType::Stop:
    return "STP";
  case midas::ExecutionType::Limit:
  default:
    return "LMT";
  }
}
inline std::string orderActionFromDirection(midas::OrderDirection direction) {
  switch (direction) {

  case midas::OrderDirection::BUY:
    return "BUY";
  case midas::OrderDirection::SELL:
  default:
    return "SELL";
  }
}

struct TransformationVisitor : public midas::OrderVisitor {
  std::list<std::shared_ptr<ibkr::internal::NativeOrder>> &ibkrOrders;
  std::atomic<OrderId> &orderCounter;
  TransformationVisitor(
      std::list<std::shared_ptr<ibkr::internal::NativeOrder>> &ibkrOrder,
      std::atomic<OrderId> &orderCounter)
      : ibkrOrders(ibkrOrder), orderCounter(orderCounter) {}
  virtual void visit(midas::SimpleOrder &order) override {
    Order nativeOrder;
    nativeOrder.totalQuantity = DecimalFunctions::doubleToDecimal(order.requestedQuantity) ;
    nativeOrder.orderType = orderType(order.execType);
    nativeOrder.action = orderActionFromDirection(order.direction);
    nativeOrder.lmtPrice = order.targetPrice;
    nativeOrder.orderId = orderCounter.fetch_add(1);
    nativeOrder.transmit = false;
    nativeOrder.tif = "GTC"; // for now all our orders are GTC. We may want to
                             // make it configurable on midas orders later
    nativeOrder.rule80A = "I";
    ibkrOrders.emplace_back(
        std::make_shared<ibkr::internal::NativeOrder>(nativeOrder, order));
  }
  virtual void visit(midas::BracketedOrder &order) override {

    order.getEntryOrder().visit(*this);
    auto parentItr = std::prev(ibkrOrders.end());
    auto parentId = (*parentItr)->nativeOrder.orderId;
    order.getProfitTakerOrder().visit(*this);
    order.getStopOrder().visit(*this);
    std::ranges::for_each(
        std::next(parentItr), std::end(ibkrOrders),
        [parentId](std::shared_ptr<ibkr::internal::NativeOrder> &order) {
          order->nativeOrder.parentId = parentId;
        });
    ibkrOrders.back()->nativeOrder.transmit = true;
  }
};

std::list<std::shared_ptr<ibkr::internal::NativeOrder>>
ibkr::internal::transformOrder(midas::Order &order,
                               std::atomic<OrderId> &orderCtr) {
  std::list<std::shared_ptr<NativeOrder>> transformed;
  TransformationVisitor transformer(transformed, orderCtr);
  order.visit(transformer);
  return transformed;
}