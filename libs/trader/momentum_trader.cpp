#include "trader/momentum_trader.hpp"
#include "broker-interface/order.hpp"
#include "logging/logging.hpp"
#include "ta_func.h"

#include <algorithm>
#include <array>
#include <boost/date_time/posix_time/time_formatters.hpp>
#include <cmath>
#include <memory>
#include <string>
#include <vector>

using namespace midas::trader;

MomentumTrader::MomentumTrader(
    std::shared_ptr<midas::DataStream> source,
    std::shared_ptr<midas::OrderManager> orderManager,
    midas::InstrumentEnum instrument,
    std::shared_ptr<logging::thread_safe_logger_t> logger)
    : Trader(100, 120, source, orderManager, logger), instrument(instrument) {
  closePrices.reserve(data.lookBackSize);
  volumes.reserve(data.lookBackSize);
  highs.reserve(data.lookBackSize);
  lows.reserve(data.lookBackSize);
  opens.reserve(data.lookBackSize);
  trades.reserve(data.lookBackSize);
  timestamps.reserve(data.lookBackSize);
}

void MomentumTrader::decide() {
  if (!data.ok() || hasOpenPosition() || paused()) {
    Trader::decision_params_t decisionParams{
      {"Not enough data", true}
    };
    decisionParamsSignal(decisionParams);
    // We do not have enough data to satisfy look back requirements
    return;
  }

  clearBuffers();
  data.copy(trades, highs, lows, opens, closePrices, vwaps, volumes,
            timestamps);
  TA_EMA(0, closePrices.size() - 1, closePrices.data(), fastMATimePeriod,
         &fastMAOutBeg, &fastMAOutSize, fastMa.data());
  TA_EMA(0, closePrices.size() - 1, closePrices.data(), slowMATimePeriod,
         &slowMAOutBeg, &slowMAOutSize, slowMa.data());

  TA_RSI(0, closePrices.size() - 1, closePrices.data(), rsiTimePeriod,
         &rsiOutBegin, &rsiOutSize, rsi.data());

  TA_SMA(0, volumes.size() - 1, volumes.data(), volumeMATimePeriod,
         &volumeMAOutBegin, &volumeMAOutSize, volumeMa.data());

  TA_ATR(0, highs.size() - 1, highs.data(), lows.data(), closePrices.data(),
         atrTimePeriod, &atrOutBegin, &atrOutSize, atr.data());
  TA_MACD(0, closePrices.size() - 1, closePrices.data(), macdFastPeriod,
          macdSlowPeriod, macdSignalPeriod, &macdOutBegin, &macdOutSize,
          macd.data(), macdSignal.data(), macdHistogram.data());
  TA_SMA(0, atrOutSize - 1, atr.data(), atrSmoothingPeriod, &atrMAOutBegin,
         &atrMAOutSize, atrMA.data());

  bool bullishMa = fastMa[fastMAOutSize - 1] > slowMa[slowMAOutSize - 1];
  bool bullishRsi = (rsi[rsiOutSize - 1] < 70) && (rsi[rsiOutSize - 1] > 45);
  bool bullishVolume = volumes.back() > volumeMa[volumeMAOutSize - 1];
  bool bullishMacd = macd[macdOutSize - 1] > macdSignal[macdOutSize - 1];
  double currentAtr = atrMA[atrMAOutSize - 1];
  double baseMultiplier = 1.5;
  double normalizedAtr = (currentAtr / closePrices.back()) * 100;
  double atrMultiplier = baseMultiplier;
  if (normalizedAtr >= 3.0) {
    atrMultiplier *= 0.5;
  } else if (normalizedAtr <= 1.0) {
    atrMultiplier *= 1.5;
  }
  double entryPrice =
      std::round(closePrices.back() * roundingCoeff) / roundingCoeff;
  double takeProfitLimit = entryPrice + atrMultiplier * currentAtr;
  double stopLossLimit = entryPrice - 2 * atrMultiplier * currentAtr;
  stopLossLimit = std::min(stopLossLimit, entryPrice - 100);
  takeProfitLimit = std::round(takeProfitLimit * roundingCoeff) / roundingCoeff;
  stopLossLimit = std::round(stopLossLimit * roundingCoeff) / roundingCoeff;

  const double commissionEstimate =
      commissionEstimatePerUnit * entryQuantity * 2;
  bool coversCommission = commissionEstimate < 2 * currentAtr;
  if (bullishMa & bullishRsi & bullishMacd & bullishVolume) {
    bullishCandles++;
  } else {
    bullishCandles = 0;
  }

  bool bullishCandleSequence = bullishCandles >= 5;

  Trader::decision_params_t decisionParams{
      {"covers comission", coversCommission},
      {"Bullish MA", bullishMa},
      {"Bullish MACD", bullishMacd},
      {"Bullish RSI", bullishRsi},
      {"Bullish Volume", bullishVolume},
      {"5 or more bullish candles", bullishCandleSequence}};
  decisionParamsSignal(decisionParams);

  if (coversCommission & bullishMa & bullishRsi & bullishVolume & bullishMacd &
      bullishCandleSequence) {
    INFO_LOG(*logger) << "entering bracket atr: " << currentAtr
                      << " bar time: " << timestamps.back();
    enterBracket(instrument, entryQuantity, midas::OrderDirection::BUY,
                 entryPrice, stopLossLimit, takeProfitLimit);
  }
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