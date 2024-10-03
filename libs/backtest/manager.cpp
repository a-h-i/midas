#include "backtest_order_manager.hpp"

bool midas::backtest::BacktestOrderManager::hasActiveOrders() const {
  return !activeOrdersList.empty();
}