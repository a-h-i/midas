//
// Created by potato on 28-12-2024.
//

#include "trader/mean_reversion_trader.hpp"

#include <ta_func.h>

using namespace midas::trader;

MeanReversionTrader::MeanReversionTrader(
    const std::shared_ptr<midas::DataStream> &source,
    const std::shared_ptr<midas::OrderManager> &orderManager,
    midas::InstrumentEnum instrument, std::size_t entryQuantity,
    const std::shared_ptr<logging::thread_safe_logger_t> &logger)
    : MeanReversionTrader(100, source, orderManager, instrument, entryQuantity,
                          logger) {}

MeanReversionTrader::MeanReversionTrader(
    std::size_t bufferSize, const std::shared_ptr<midas::DataStream> &source,
    const std::shared_ptr<midas::OrderManager> &orderManager,
    midas::InstrumentEnum instrument, std::size_t entryQuantity,
    const std::shared_ptr<logging::thread_safe_logger_t> &logger)
    : Trader(bufferSize, 120, source, orderManager, logger),
      instrument(instrument), entryQuantity(entryQuantity) {}

void MeanReversionTrader::clearBuffers() {
  closePrices.clear();
  volumes.clear();
  highs.clear();
  highs.clear();
  lows.clear();
  opens.clear();
  trades.clear();
  timestamps.clear();
};

std::pair<double, double> MeanReversionTrader::decideProfitAndStopLossLevels(
    double entryPrice [[maybe_unused]], OrderDirection direction) {
  double profitTaker = bbMiddle.back();
  double stopLoss;
  if (direction == OrderDirection::BUY) {
    stopLoss = bbLower.back() - 2;
    if (stopLoss >= entryPrice) {
      stopLoss = entryPrice - bbUpper.back() - bbMiddle.back() - 0.5;
    }
    if (profitTaker <= entryPrice) {
      profitTaker = entryPrice + bbUpper.back() - bbMiddle.back() + 0.5;
    }
  } else {
    stopLoss = bbUpper.back() + 2;
    if (stopLoss <= entryPrice) {
      stopLoss = entryPrice + bbUpper.back() - bbMiddle.back() + 0.5;
    }
    if (profitTaker >= entryPrice) {
      profitTaker = entryPrice - bbUpper.back() - bbMiddle.back() - 0.5;
    }
  }
  profitTaker = std::round(profitTaker * roundingCoeff) / roundingCoeff;
  stopLoss = std::round(stopLoss * roundingCoeff) / roundingCoeff;
  return {profitTaker, stopLoss};
}

MeanReversionTrader::candle_decision_t
MeanReversionTrader::decideCandle(std::size_t candleEndOffset) {
  const int sizeOffset = -1 - candleEndOffset;
  bool aboveUpper =
      closePrices[closePrices.size() + sizeOffset] >= bbUpper[bbUpper.size() + sizeOffset];
  bool belowLower =
      closePrices[closePrices.size() + sizeOffset] <= bbLower[bbLower.size() + sizeOffset];

  double bullishIndicator = static_cast<double>(belowLower);
  double bearishIndicator = static_cast<double>(aboveUpper);

  return {
      .bullishIndicator = bullishIndicator,
      .bearishIndicator = bearishIndicator,
      .maxBullish = 1,
      .maxBearish = 1
  };
}

std::string MeanReversionTrader::traderName() const {
  return "MeanReversionTrader";
}

std::size_t MeanReversionTrader::decideEntryQuantity() { return entryQuantity; }

double MeanReversionTrader::decideEntryPrice() {
  double entryPrice = closePrices.back();
  return std::round(entryPrice * roundingCoeff) / roundingCoeff;
}

void MeanReversionTrader::calculateTechnicalAnalysis() {
  clearBuffers();
  data.copy(trades, highs, lows, opens, closePrices, vwaps, volumes,
            timestamps);
  TA_BBANDS(0, closePrices.size() - 1, closePrices.data(), 20, 2.0, 2.0, TA_MAType_SMA,
            &bbBegIndex, &bbOutSize, bbUpper.data(), bbMiddle.data(),
            bbLower.data());
}

void MeanReversionTrader::decide() {
  if (!data.ok() || hasOpenPosition() || paused()) {
    std::string statusString;
    if (hasOpenPosition()) {
      statusString = "Maintaining position";

    } else if (paused()) {
      statusString = "Paused";
    } else {
      statusString = "Not enough data";
    }
    Trader::decision_params_t decisionParams{{statusString, "true"}};
    decisionParamsSignal(decisionParams);
    // We do not have enough data to satisfy look back requirements
    return;
  }
  calculateTechnicalAnalysis();

  double entryPrice = decideEntryPrice();
  const std::size_t entryQuantity = decideEntryQuantity();
  candle_decision_t candle_decision = decideCandle(0);
  Trader::decision_params_t decisionParams{
      {{"Bullish Indicator", std::to_string(candle_decision.bullishIndicator)},
       {"Bearish Indicator",
        std::to_string(candle_decision.bearishIndicator)}}};
  decisionParamsSignal(decisionParams);

  bool enterLong =
      candle_decision.bullishIndicator == candle_decision.maxBullish;
  bool enterShort =
      candle_decision.bearishIndicator == candle_decision.maxBearish;

  if (enterLong) {
    INFO_LOG(*logger) << "entering long bracket "
                      << " bar time: " << timestamps.back();
    const auto bracketBoundaries =
        decideProfitAndStopLossLevels(entryPrice, OrderDirection::BUY);
    enterBracket(instrument, entryQuantity, midas::OrderDirection::BUY,
                 entryPrice, bracketBoundaries.second, bracketBoundaries.first);
  } else if (enterShort) {
    INFO_LOG(*logger) << "entering short bracket "
                      << " bar time: " << timestamps.back();
    const auto bracketBoundaries =
        decideProfitAndStopLossLevels(entryPrice, OrderDirection::SELL);
    enterBracket(instrument, entryQuantity, midas::OrderDirection::SELL,
                 entryPrice, bracketBoundaries.second, bracketBoundaries.first);
  }
}
