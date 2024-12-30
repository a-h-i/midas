#include "trader/momentum_trader.hpp"
#include "broker-interface/order.hpp"
#include "logging/logging.hpp"
#include "ta_func.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <memory>
#include <string>
#include <vector>

using namespace midas::trader;

MomentumTrader::MomentumTrader(
    const std::shared_ptr<midas::DataStream> &source,
    const std::shared_ptr<midas::OrderManager> &orderManager,
    midas::InstrumentEnum instrument, std::size_t entryQuantity,
    const std::shared_ptr<logging::thread_safe_logger_t> &logger)
    : MomentumTrader(100, source, orderManager, instrument, entryQuantity,
                     logger) {}

MomentumTrader::MomentumTrader(
    std::size_t bufferSize, const std::shared_ptr<midas::DataStream> &source,
    const std::shared_ptr<midas::OrderManager> &orderManager,
    midas::InstrumentEnum instrument, std::size_t entryQuantity,
    const std::shared_ptr<logging::thread_safe_logger_t> &logger)
    : Trader(bufferSize, 5, source, orderManager, logger),
      instrument(instrument), entryQuantity(entryQuantity) {
  closePrices.reserve(data.lookBackSize);
  volumes.reserve(data.lookBackSize);
  highs.reserve(data.lookBackSize);
  lows.reserve(data.lookBackSize);
  opens.reserve(data.lookBackSize);
  trades.reserve(data.lookBackSize);
  timestamps.reserve(data.lookBackSize);
}

void MomentumTrader::calculateTechnicalAnalysis() {
  clearBuffers();
  data.copy(trades, highs, lows, opens, closePrices, vwaps, volumes,
            timestamps);
  TA_EMA(0, closePrices.size() - 1, closePrices.data(), fastMATimePeriod,
         &fastMAOutBeg, &fastMAOutSize, fastMa.data());
  TA_EMA(0, closePrices.size() - 1, closePrices.data(), slowMATimePeriod,
         &slowMAOutBeg, &slowMAOutSize, slowMa.data());

  TA_SMA(0, volumes.size() - 1, volumes.data(), volumeMATimePeriod,
         &volumeMAOutBegin, &volumeMAOutSize, volumeMa.data());

  TA_ATR(0, highs.size() - 1, highs.data(), lows.data(), closePrices.data(),
         atrTimePeriod, &atrOutBegin, &atrOutSize, atr.data());
  TA_MACD(0, closePrices.size() - 1, closePrices.data(), macdFastPeriod,
          macdSlowPeriod, macdSignalPeriod, &macdOutBegin, &macdOutSize,
          macd.data(), macdSignal.data(), macdHistogram.data());
  TA_EMA(0, atrOutSize - 1, atr.data(), atrSmoothingPeriod, &atrMAOutBegin,
         &atrMAOutSize, atrMA.data());
  TA_RSI(0, closePrices.size() - 1, closePrices.data(), rsiTimePeriod,
         &rsiOutBegin, &rsiOutSize, rsi.data());
  TA_BBANDS(0, closePrices.size() - 1, closePrices.data(), 20, 2.0, 2.0,
            TA_MAType_SMA, &bbBegIndex, &bbOutSize, bbUpper.data(),
            bbMiddle.data(), bbLower.data());
}

MomentumTrader::candle_decision_t
MomentumTrader::decideCandle(std::size_t candleEndOffset) {
  int sizeOffset = -1 - candleEndOffset;
  
  bool bullishMA = fastMa[fastMAOutSize + sizeOffset] >
                   slowMa[slowMAOutSize + sizeOffset];
  bool bullishMACD = macd[macdOutSize + sizeOffset] >
                     macdSignal[macdOutSize + sizeOffset];
  bool bullishRSI = rsi[rsiOutSize + sizeOffset] < 65;

  bool bearishMA = fastMa[fastMAOutSize + sizeOffset] <
                   slowMa[slowMAOutSize + sizeOffset];
  bool bearishMACD = macd[macdOutSize + sizeOffset] <
                     macdSignal[macdOutSize + sizeOffset];
  bool bearishRSI = rsi[rsiOutSize + sizeOffset] > 25;

  bool volumeAcceptable = volumes[volumes.size() + sizeOffset] >
                          volumeMa[volumeMAOutSize + sizeOffset];
  bool aboveUpper = closePrices[closePrices.size() + sizeOffset] >= bbUpper[bbOutSize + sizeOffset];
  bool belowLower = closePrices[closePrices.size() + sizeOffset] <= bbLower[bbOutSize + sizeOffset];
  
  double bullishIndicator = static_cast<double>(bullishMA) + bullishMACD +
                            bullishRSI + volumeAcceptable;
  double bearishIndicator = static_cast<double>(bearishMA) + bearishMACD +
                            bearishRSI + volumeAcceptable;
  if (aboveUpper) {
    bearishIndicator += 1;
    bullishIndicator = 0;
  }
  if (belowLower) {
    bearishIndicator = 0;
    bullishIndicator += 1;
  }



  if (bullishIndicator >= 4) {
    ++bullishCandlesinARow;
  } else {
    bullishCandlesinARow.store(0);
  }
  if (bearishIndicator >= 4) {
    ++bearishCandlesInARow;
  } else {
    bearishCandlesInARow.store(0);
  }

  bullishIndicator += static_cast<double>(bullishCandlesinARow) / 3;
  bearishIndicator += static_cast<double>(bearishCandlesInARow) / 3;

  return {
      .bullishIndicator = bullishIndicator,
      .bearishIndicator = bearishIndicator,
      .maxBullish = 5,
      .maxBearish = 5,
  };
}

void MomentumTrader::decide() {
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
  candle_decision_t candle_decision;
  for (int i = 5; i >= 0; i -= 1) {
    candle_decision = decideCandle(i);
  }

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

double MomentumTrader::decideEntryPrice() {
  double entryPrice = opens.back() + (highs.back() - lows.back()) / 2;
  return std::round(entryPrice * roundingCoeff) / roundingCoeff;
}

std::size_t MomentumTrader::decideEntryQuantity() { return entryQuantity; }

std::pair<double, double>
MomentumTrader::decideProfitAndStopLossLevels(double entryPrice,
                                              OrderDirection orderDirection) {
  double profitOffset = 5;
  double stopLossOffset = -15;
  if (orderDirection == OrderDirection::SELL) {
    profitOffset *= -1;
    stopLossOffset *= -1;
  }

  double takeProfitLimit = entryPrice + profitOffset;
  double stopLossLimit = entryPrice + stopLossOffset;

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

void MomentumTrader::clearBuffers() {

  closePrices.clear();
  volumes.clear();
  highs.clear();
  highs.clear();
  lows.clear();
  opens.clear();
  trades.clear();
  timestamps.clear();
}

std::string MomentumTrader::traderName() const {
  return std::string("Momentum trader - ") + instrument;
}
