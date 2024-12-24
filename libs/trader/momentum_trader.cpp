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
    midas::InstrumentEnum instrument,
    const std::shared_ptr<logging::thread_safe_logger_t> &logger)
    : MomentumTrader(100, source, orderManager, instrument, logger) {}

MomentumTrader::MomentumTrader(
    std::size_t bufferSize, const std::shared_ptr<midas::DataStream> &source,
    const std::shared_ptr<midas::OrderManager> &orderManager,
    midas::InstrumentEnum instrument,
    const std::shared_ptr<logging::thread_safe_logger_t> &logger)
    : Trader(bufferSize, 5, source, orderManager, logger),
      instrument(instrument) {
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
  TA_SMA(0, atrOutSize - 1, atr.data(), atrSmoothingPeriod, &atrMAOutBegin,
         &atrMAOutSize, atrMA.data());
  TA_RSI(0, closePrices.size() - 1, closePrices.data(), rsiTimePeriod,
         &rsiOutBegin, &rsiOutSize, rsi.data());
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
    Trader::decision_params_t decisionParams{{statusString, true}};
    decisionParamsSignal(decisionParams);
    // We do not have enough data to satisfy look back requirements
    return;
  }

  calculateTechnicalAnalysis();

  bool bullishMA = fastMa[fastMAOutSize - 1] > slowMa[slowMAOutSize - 1];
  bool bullishMACD = macd[macdOutSize - 1] > macdSignal[macdOutSize - 1];
  bool bullishRSI = rsi[rsiOutSize - 1] < 65;

  bool bearishMA = fastMa[fastMAOutSize - 1] < slowMa[slowMAOutSize - 1];
  bool bearishMACD = macd[macdOutSize - 1] < macdSignal[macdOutSize - 1];
  bool bearishRSI = rsi[rsiOutSize - 1] > 25;

  double currentAtr = atrMA[atrMAOutSize - 1];
  double normalizedAtr = (currentAtr / closePrices.back()) * 100;

  bool normalizedAtrAcceptable = normalizedAtr > 1.0;
  bool volumeAcceptable = volumes.back() > volumeMa[volumeMAOutSize - 1];

  double entryPrice = (highs.back() - lows.back()) * 0.7 + lows.back();
  entryPrice = std::round(entryPrice * roundingCoeff) / roundingCoeff;

  const std::size_t entryQuantity = decideEntryQuantity();
  const double commissionEstimate =
      commissionEstimatePerUnit * entryQuantity * 2;
  bool coversCommission = commissionEstimate < 2 * currentAtr;
  Trader::decision_params_t decisionParams{
      {"covers comission", coversCommission},
      {"Bullish MA", bullishMA},
      {"Bullish MACD", bullishMACD},
      {"Bullish Volume", volumeAcceptable},
      {"Bullish RSI", bullishRSI},
      {"Bearish MA", bearishMA},
      {"Bearish MACD", bearishMACD},
      {"Bearish RSI", bearishRSI},
      {"ATR acceptable", normalizedAtrAcceptable}};
  decisionParamsSignal(decisionParams);

  double bullishIndicator = static_cast<double>(bullishMA) + bullishMACD +
                            bullishRSI + volumeAcceptable +
                            normalizedAtrAcceptable + coversCommission;
  double bearishIndicator = static_cast<double>(bearishMA) + bearishMACD +
                            bearishRSI + volumeAcceptable +
                            normalizedAtrAcceptable + coversCommission;
  bool enterLong = bullishIndicator == 6;
  bool enterShort = bearishIndicator == 6;

  if (enterLong) {
    INFO_LOG(*logger) << "entering long bracket atr: " << currentAtr
                      << " bar time: " << timestamps.back();
    const auto bracketBoundaries =
        decideProfitAndStopLossLevels(entryPrice, OrderDirection::BUY);
    enterBracket(instrument, entryQuantity, midas::OrderDirection::BUY,
                 entryPrice, bracketBoundaries.second, bracketBoundaries.first);
  } else if (enterShort) {
    const auto bracketBoundaries =
        decideProfitAndStopLossLevels(entryPrice, OrderDirection::SELL);
    enterBracket(instrument, entryQuantity, midas::OrderDirection::SELL,
                 entryPrice, bracketBoundaries.second, bracketBoundaries.first);
  }
}

std::size_t MomentumTrader::decideEntryQuantity() { return 2; }

std::pair<double, double>
MomentumTrader::decideProfitAndStopLossLevels(double entryPrice,
                                              OrderDirection orderDirection) {
  double profitOffset = 10;
  double stopLossOffset = -50;
  if (orderDirection == OrderDirection::SELL) {
    profitOffset *= -1;
    stopLossOffset *= -1;
  }

  double takeProfitLimit = entryPrice + profitOffset;
  double stopLossLimit = entryPrice + stopLossOffset;

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
