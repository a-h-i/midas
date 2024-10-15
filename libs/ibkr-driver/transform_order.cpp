#include "CommonDefs.h"
#include "broker-interface/order.hpp"
#include "ibkr/internal/build_contracts.hpp"
#include "ibkr/internal/ibkr_order_manager.hpp"
#include <algorithm>
#include <iterator>

inline std::string orderType(midas::ExecutionType execution) {
  switch (execution) {

  case midas::ExecutionType::Limit:
    return "LMT";
  case midas::ExecutionType::Stop:
    return "STP";
  }
}
inline std::string orderActionFromDirection(midas::OrderDirection direction) {
  switch (direction) {

  case midas::OrderDirection::BUY:
    return "BUY";
  case midas::OrderDirection::SELL:
    return "SELL";
  }
}

struct TransformationVisitor : public midas::OrderVisitor {
  std::list<ibkr::internal::OrderInfo> &ibkrOrders;
  std::atomic<OrderId> &orderCounter;
  TransformationVisitor(std::list<ibkr::internal::OrderInfo> &ibkrOrder,
                        std::atomic<OrderId> &orderCounter)
      : ibkrOrders(ibkrOrder), orderCounter(orderCounter) {}
  virtual void visit(midas::SimpleOrder &order) override {
    auto &ibkrOrder = ibkrOrders.emplace_back();
    ibkrOrder.ibkrOrder.totalQuantity = order.requestedQuantity;
    ibkrOrder.ibkrOrder.orderType = orderType(order.execType);
    ibkrOrder.ibkrOrder.action = orderActionFromDirection(order.direction);
    ibkrOrder.ibkrOrder.lmtPrice = order.targetPrice;
    ibkrOrder.ibkrOrder.orderId = orderCounter.fetch_add(1);
    ibkrOrder.ibkrOrder.transmit = false;
    ibkrOrder.ibkrOrder.tif =
        "GTC"; // for now all our orders are GTC. We may want to
               // make it configurable on midas orders later
    ibkrOrder.ibkrOrder.rule80A = "I";
    ibkrOrder.instrument = order.instrument;
    ibkrOrder.ibkrContract =
        ibkr::internal::build_futures_contract(order.instrument);
  }
  virtual void visit(midas::BracketedOrder &order) override {

    order.getEntryOrder().visit(*this);
    auto parentItr = std::prev(ibkrOrders.end());
    auto parentId = parentItr->ibkrOrder.orderId;
    order.getProfitTakerOrder().visit(*this);
    order.getStopOrder().visit(*this);
    std::ranges::for_each(std::next(parentItr), std::end(ibkrOrders),
                          [parentId](ibkr::internal::OrderInfo &order) {
                            order.ibkrOrder.parentId = parentId;
                          });
    ibkrOrders.back().ibkrOrder.transmit = true;
  }
};

std::list<ibkr::internal::OrderInfo>
ibkr::internal::transformOrder(midas::Order &order,
                               std::atomic<OrderId> &orderCtr) {
  std::list<OrderInfo> transformed;
  TransformationVisitor transformer(transformed, orderCtr);
  order.visit(transformer);
  return transformed;
}