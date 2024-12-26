//
// Created by potato on 24-12-2024.
//
#include "trader/momentum_trader.hpp"


std::size_t midas::trader::StockMomentumTrader::decideEntryQuantity() {
  return entryQuantity;
}

std::pair<double, double>
  midas::trader::StockMomentumTrader::decideProfitAndStopLossLevels(double entryPrice, midas::OrderDirection orderDirection) {
  double profitOffset = 0.75;
  double stopOffset = -2.25;
  if (orderDirection == midas::OrderDirection::SELL) {
    profitOffset *= -1;
    stopOffset *= -1;
  }
  int takeProfitLimit = entryPrice + profitOffset;
  int stopLossLimit = entryPrice + stopOffset;
  takeProfitLimit = std::round(takeProfitLimit * roundingCoeff) / roundingCoeff;
  stopLossLimit = std::round(stopLossLimit * roundingCoeff) / roundingCoeff;
  return {takeProfitLimit, stopLossLimit};
}