#include "broker-interface/order.hpp"
#include "ta_func.h"
#include "trader/trader.hpp"
#include <array>
#include <cmath>
/**
 * Very simple momentum trader
 * Intended to scalp MNQ and MES and similar indexes.
 */
class MomentumTrader : public midas::trader::Trader {
  static constexpr int fastMATimePeriod = 9;
  static constexpr int slowMATimePeriod = 100;
  static constexpr int volumeMATimePeriod = 10;
  static constexpr int atrTimePeriod = 9;
  static constexpr int rsiTimePeriod = 9;
  static constexpr int entryQuantity = 2;
  static constexpr int macdFastPeriod = 6, macdSlowPeriod = 13,
                       macdSignalPeriod = 5;
  static constexpr double commissionEstimatePerUnit = 0.25;
  static constexpr double roundingCoeff = 4; // minimum of .25 index moves
  std::array<double, 100> slowMa, fastMa, rsi, volumeMa, atr, macd, macdSignal,
      macdHistogram;
  int slowMAOutBeg = 0, fastMAOutBeg = 0, rsiOutBegin = 0, volumeMAOutBegin = 0,
      atrOutBegin = 0, macdOutBegin = 0, slowMAOutSize = 0, fastMAOutSize,
      rsiOutSize = 0, volumeMAOutSize = 0, atrOutSize = 0, macdOutSize = 0;
  const midas::InstrumentEnum instrument;

public:
  MomentumTrader(std::shared_ptr<midas::DataStream> source,
                 std::shared_ptr<midas::OrderManager> orderManager,
                 midas::InstrumentEnum instrument)
      : Trader(100, 120, source, orderManager), instrument(instrument) {}
  virtual ~MomentumTrader() {}
  virtual void decide() override {
    if (!data.ok() || hasOpenPosition()) {
      // We do not have enough data to satisfy look back requirements
      return;
    }

    auto &closePrices = data.closesBuffer();
    auto &volumes = data.volumesBuffer();
    auto &highs = data.highsBuffer();
    auto &lows = data.lowsBuffer();

    TA_EMA(0, closePrices.size() - 1, closePrices.linearize(), fastMATimePeriod,
           &fastMAOutBeg, &fastMAOutSize, fastMa.data());
    TA_EMA(0, closePrices.size() - 1, closePrices.linearize(), slowMATimePeriod,
           &slowMAOutBeg, &slowMAOutSize, slowMa.data());

    TA_RSI(0, closePrices.size(), closePrices.linearize(), rsiTimePeriod,
           &rsiOutBegin, &rsiOutSize, rsi.data());

    TA_SMA(0, volumes.size() - 1, volumes.linearize(), volumeMATimePeriod,
           &volumeMAOutBegin, &volumeMAOutSize, volumeMa.data());

    TA_ATR(0, highs.size(), highs.linearize(), lows.linearize(),
           closePrices.linearize(), atrTimePeriod, &atrOutBegin, &atrOutSize,
           atr.data());
    TA_MACD(0, closePrices.size(), closePrices.linearize(), macdFastPeriod,
            macdSlowPeriod, macdSignalPeriod, &macdOutBegin, &macdOutSize,
            macd.data(), macdSignal.data(), macdHistogram.data());

    bool bullishMa = fastMa[fastMAOutSize - 1] > slowMa[slowMAOutSize - 1];
    bool bullishRsi = (rsi[rsiOutSize - 1] < 70) & (rsi[rsiOutSize - 1] > 45);
    bool bullishVolume = volumes.back() > volumeMa[volumeMAOutSize - 1];
    bool bullishMacd = macd[macdOutSize - 1] > macdSignal[macdOutSize - 1];
    double takeProfitLimit = closePrices.back() + 2 * atr.back();
    double stopPriceLimit = closePrices.back() - 2 * atr.back();
    takeProfitLimit =
        std::round(takeProfitLimit * roundingCoeff) / roundingCoeff;
    stopPriceLimit = std::round(stopPriceLimit * roundingCoeff) / roundingCoeff;

    const double commissionEstimate =
        commissionEstimatePerUnit * entryQuantity * 2;
    bool coversCommission = commissionEstimate < 2 * atr.back();

    if (coversCommission & bullishMa & bullishRsi & bullishVolume &
        bullishMacd) {
      buy(entryQuantity, instrument);
    }
  }
};

std::unique_ptr<midas::trader::Trader> midas::trader::momentumExploit(
    std::shared_ptr<midas::DataStream> source,
    std::shared_ptr<midas::OrderManager> orderManager, InstrumentEnum instrument ) {

  return std::make_unique<MomentumTrader>(source, orderManager, instrument);
}