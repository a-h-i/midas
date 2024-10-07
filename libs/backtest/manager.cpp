#include "backtest_order_manager.hpp"
#include "broker-interface/order.hpp"
#include "data/bar.hpp"
#include <algorithm>
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

  std::ranges::for_each(
      activeOrdersList, [&bar](std::shared_ptr<Order> &orderPtr) {
        SimulationOrderTransmitter transmitter(
            bar, [&orderPtr](double price, double commission,
                             [[maybe_unused]] bool isFinished) {
              orderPtr->setFilled(price, commission,
                                  orderPtr->requestedQuantity);
            });
        orderPtr->transmit(transmitter);
      });
  for (auto it = activeOrdersList.begin(); it != activeOrdersList.end(); it++) {
    if (it->get()->isDone()) {
      auto copy = std::prev(it);
      completedOrdersList.splice(completedOrdersList.end(), activeOrdersList, it);
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

static bool transmitHelper(const midas::SimpleOrder &order,
                           const midas::Bar *bar) {
  const midas::OrderDirection direction =
      order.execType == midas::ExecutionType::Stop ? ~order.direction
                                                   : order.direction;
  return isTriggered(direction, order.targetPrice, bar);
}

void midas::backtest::SimulationOrderTransmitter::transmit(
    midas::SimpleOrder &order) {
  if (transmitHelper(order, bar)) {
    triggerCallback(order.targetPrice, 0.25 * order.requestedQuantity, true);
  }
}

void midas::backtest::SimulationOrderTransmitter::transmit(
    midas::BracketedOrder &order) {
  // first check if parent has not executed
  if (order.state() == OrderStatusEnum::Accepted &&
      transmitHelper(order.getEntryOrder(), bar)) {
    triggerCallback(order.getEntryOrder().targetPrice,
                    0.25 * order.getEntryOrder().requestedQuantity, false);
  } else if (order.state() == OrderStatusEnum::WaitingForChildren) {
    // first we check the stopLoss Order
    if (transmitHelper(order.getStopOrder(), bar)) {
      triggerCallback(order.getStopOrder().targetPrice,
                      0.25 * order.getStopOrder().requestedQuantity, true);
    } else if (transmitHelper(order.getProfitTakerOrder(), bar)) {
      triggerCallback(order.getProfitTakerOrder().targetPrice,
                      0.25 * order.getProfitTakerOrder().requestedQuantity,
                      true);
    }
  }
}