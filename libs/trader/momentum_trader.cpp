#include "trader/momentum_trader.hpp"
#include "broker-interface/order.hpp"
#include "logging/logging.hpp"
#include "ta_func.h"

#include <array>
#include <cmath>
#include <memory>
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
}

void MomentumTrader::decide() {
  if (!data.ok() || hasOpenPosition()) {
    // We do not have enough data to satisfy look back requirements
    return;
  }

  clearBuffers();
  data.copy(trades, highs, lows, opens, closePrices, vwaps, volumes);
  TA_EMA(0, closePrices.size() - 1, closePrices.data(), fastMATimePeriod,
         &fastMAOutBeg, &fastMAOutSize, fastMa.data());
  TA_EMA(0, closePrices.size() - 1, closePrices.data(), slowMATimePeriod,
         &slowMAOutBeg, &slowMAOutSize, slowMa.data());

  TA_RSI(0, closePrices.size(), closePrices.data(), rsiTimePeriod, &rsiOutBegin,
         &rsiOutSize, rsi.data());

  TA_SMA(0, volumes.size() - 1, volumes.data(), volumeMATimePeriod,
         &volumeMAOutBegin, &volumeMAOutSize, volumeMa.data());

  TA_ATR(0, highs.size(), highs.data(), lows.data(), closePrices.data(),
         atrTimePeriod, &atrOutBegin, &atrOutSize, atr.data());
  TA_MACD(0, closePrices.size(), closePrices.data(), macdFastPeriod,
          macdSlowPeriod, macdSignalPeriod, &macdOutBegin, &macdOutSize,
          macd.data(), macdSignal.data(), macdHistogram.data());

  bool bullishMa = fastMa[fastMAOutSize - 1] > slowMa[slowMAOutSize - 1];
  bool bullishRsi = (rsi[rsiOutSize - 1] < 70) && (rsi[rsiOutSize - 1] > 45);
  bool bullishVolume = volumes.back() > volumeMa[volumeMAOutSize - 1];
  bool bullishMacd = macd[macdOutSize - 1] > macdSignal[macdOutSize - 1];
  double currentAtr = atr[atrOutSize - 1];
  double takeProfitLimit = closePrices.back() + 2 * currentAtr;
  double stopLossLimit = closePrices.back() - 2 * currentAtr;
  double entryPrice =
      std::round(closePrices.back() * roundingCoeff) / roundingCoeff;
  takeProfitLimit = std::round(takeProfitLimit * roundingCoeff) / roundingCoeff;
  stopLossLimit = std::round(stopLossLimit * roundingCoeff) / roundingCoeff;

  const double commissionEstimate =
      commissionEstimatePerUnit * entryQuantity * 2;
  bool coversCommission = commissionEstimate < 2 * currentAtr;

  if (coversCommission & bullishMa & bullishRsi & bullishVolume & bullishMacd) {
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
}