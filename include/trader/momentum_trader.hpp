#pragma once
#include "trader/base_trader.hpp"
#include <vector>

namespace midas::trader {
/**
 * Very simple momentum trader
 * Intended to scalp MNQ and MES and similar indexes.
 */

class MomentumTrader : public midas::trader::Trader {
  static constexpr int fastMATimePeriod = 9;
  static constexpr int slowMATimePeriod = 100;
  static constexpr int volumeMATimePeriod = 10;
  static constexpr int atrTimePeriod = 13;
  static constexpr int rsiTimePeriod = 9;
  static constexpr int entryQuantity = 2;
  static constexpr int macdFastPeriod = 6, macdSlowPeriod = 13,
                       macdSignalPeriod = 5;
  static constexpr double commissionEstimatePerUnit = 0.25;
  static constexpr double roundingCoeff = 4; // minimum of .25 index moves
  std::array<double, 100> slowMa, fastMa, rsi, volumeMa, atr, atrMA, macd, macdSignal,
      macdHistogram;
  int slowMAOutBeg = 0, fastMAOutBeg = 0, rsiOutBegin = 0, volumeMAOutBegin = 0,
      atrOutBegin = 0, atrMAOutBegin = 0, macdOutBegin = 0, slowMAOutSize = 0, fastMAOutSize,
      rsiOutSize = 0, volumeMAOutSize = 0, atrOutSize = 0, atrMAOutSize = 0, macdOutSize = 0;
  const midas::InstrumentEnum instrument;
  std::vector<unsigned int> trades;
  std::vector<double> closePrices, volumes, highs, lows, opens, vwaps;
  std::vector<boost::posix_time::ptime> timestamps;
  unsigned int bullishCandles{0};

  void clearBuffers();

public:
  MomentumTrader(std::shared_ptr<midas::DataStream> source,
                 std::shared_ptr<midas::OrderManager> orderManager,
                 midas::InstrumentEnum instrument,
                 std::shared_ptr<logging::thread_safe_logger_t> logger);

  virtual ~MomentumTrader() = default;
  virtual void decide() override;
};
} // namespace midas::trader