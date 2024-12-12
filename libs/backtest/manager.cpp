#include "backtest_order_manager.hpp"
#include "broker-interface/order.hpp"
#include "data/bar.hpp"
#include <algorithm>
#include <list>
#include <memory>

bool midas::backtest::BacktestOrderManager::hasActiveOrders() const {
  return !activeOrdersList.empty();
}

void midas::backtest::BacktestOrderManager::transmit(
    std::shared_ptr<Order> order) {
  order->setTransmitted();
  activeOrdersList.push_back(order);
}

void midas::backtest::BacktestOrderManager::simulate(const midas::Bar *bar) {
  SimulationOrderTransmitter transmitter(bar);
  std::ranges::for_each(activeOrdersList,
                        [&transmitter](std::shared_ptr<Order> &orderPtr) {
                          orderPtr->visit(transmitter);
                        });
  for (auto it = activeOrdersList.begin(); it != activeOrdersList.end(); it++) {
    if (it->get()->isDone()) {
      auto copy = std::prev(it);
      completedOrdersList.splice(completedOrdersList.end(), activeOrdersList,
                                 it);
      it = copy;
    }
  }
}

static bool isTriggered(midas::OrderDirection direction, double targetPrice,
                        const midas::Bar *bar) {
  if (direction == midas::OrderDirection::BUY && bar->low <= targetPrice) {
    return true;
  } else if (direction == midas::OrderDirection::SELL &&
             bar->high >= targetPrice) {
    return true;
  } else {
    return false;
  }
}

static bool visitHelper(const midas::SimpleOrder &order,
                        const midas::Bar *bar) {
  const midas::OrderDirection direction =
      order.execType == midas::ExecutionType::Stop ? ~order.direction
                                                   : order.direction;
  return isTriggered(direction, order.targetPrice, bar);
}

void midas::backtest::SimulationOrderTransmitter::visit(
    midas::SimpleOrder &order) {
  if (visitHelper(order, bar)) {
    order.setFilled(order.targetPrice, 0.25 * order.requestedQuantity,
                    order.requestedQuantity);
  }
}

void midas::backtest::SimulationOrderTransmitter::visit(
    midas::BracketedOrder &order) {
  // first check if parent has not executed
  if (order.state() == OrderStatusEnum::Accepted &&
      visitHelper(order.getEntryOrder(), bar)) {
    auto &entry = order.getEntryOrder();
    entry.setFilled(entry.targetPrice, 0.25 * entry.requestedQuantity,
                    entry.requestedQuantity);
  } else if (order.state() == OrderStatusEnum::WaitingForChildren) {
    // first we check the stopLoss Order
    if (visitHelper(order.getStopOrder(), bar)) {
      auto &stop = order.getStopOrder();
      stop.setFilled(stop.targetPrice, 0.25 * stop.requestedQuantity,
                     stop.requestedQuantity);
    } else if (visitHelper(order.getProfitTakerOrder(), bar)) {
      auto &profit = order.getProfitTakerOrder();
      profit.setFilled(profit.targetPrice, 0.25 * profit.requestedQuantity,
                       profit.requestedQuantity);
    }
  }
}

std::list<midas::Order *>
midas::backtest::BacktestOrderManager::getFilledOrders() {
  std::list<midas::Order *> orderPtrs;

  for (auto &orderPtr : completedOrdersList) {
    orderPtrs.push_back(orderPtr.get());
  }
  return orderPtrs;
}