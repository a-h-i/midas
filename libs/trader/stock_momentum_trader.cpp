//
// Created by potato on 24-12-2024.
//
#include "trader/momentum_trader.hpp"


std::size_t midas::trader::StockMomentumTrader::decideEntryQuantity() {
  return entryQuantity;
}

std::pair<double, double>
  midas::trader::StockMomentumTrader::decideProfitAndStopLossLevels(double entryPrice, midas::OrderDirection orderDirection) {
  double profitOffset = 0.5;
  double stopOffset = -2.25;
  if (orderDirection == midas::OrderDirection::SELL) {
    profitOffset *= -1;
    stopOffset *= -1;
  }
  double takeProfitLimit = entryPrice + profitOffset;
  double stopLossLimit = entryPrice + stopOffset;
  if (std::fmod(takeProfitLimit, 5) == 0) {
    takeProfitLimit -= 0.25;
  }
  if (std::fmod(stopLossLimit, 5) == 0) {
    stopLossLimit += 0.25;
  };
  takeProfitLimit = std::round(takeProfitLimit * roundingCoeff) / roundingCoeff;
  stopLossLimit = std::round(stopLossLimit * roundingCoeff) / roundingCoeff;
  return {takeProfitLimit, stopLossLimit};
}

double midas::trader::StockMomentumTrader::decideEntryPrice() {
  double entryPrice = opens.back() + (highs.back() - lows.back()) / 2;
  return std::round(entryPrice * roundingCoeff) / roundingCoeff;
}