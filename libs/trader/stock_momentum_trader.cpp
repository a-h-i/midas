//
// Created by potato on 24-12-2024.
//
#include "trader/momentum_trader.hpp"


std::size_t midas::trader::StockMomentumTrader::decideEntryQuantity() {
  return 100;
}

std::pair<double, double>
  decideProfitAndStopLossLevels(double entryPrice, midas::OrderDirection orderDirection) {
  double profitOffset = 0.5;
  double stopOffset = -3.25;
  if (orderDirection == midas::OrderDirection::SELL) {
    profitOffset *= -1;
    stopOffset *= -1;
  }
  return {entryPrice + profitOffset, entryPrice + stopOffset};
}