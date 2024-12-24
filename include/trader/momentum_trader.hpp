#pragma once
#include "trader/base_trader.hpp"
#include <vector>

namespace midas::trader {
/**
 * Very simple momentum trader
 * Intended to scalp MNQ and MES and similar indexes.
 */

class MomentumTrader : public midas::trader::Trader {
public:
  static constexpr int fastMATimePeriod = 5;
  static constexpr int slowMATimePeriod = 15;
  static constexpr int volumeMATimePeriod = 15;
  static constexpr int atrTimePeriod = 13;
  static constexpr int rsiTimePeriod = 6;

  static constexpr int macdFastPeriod = 6, macdSlowPeriod = 13,
                       macdSignalPeriod = 4;
  static constexpr int atrSmoothingPeriod = 20;
  static constexpr double commissionEstimatePerUnit = 0.25;
  static constexpr double roundingCoeff = 4; // minimum of .25 index moves
private:
  std::array<double, 100> slowMa, fastMa, volumeMa, atr, atrMA, macd,
      macdSignal, macdHistogram, rsi;
  int slowMAOutBeg = 0, fastMAOutBeg = 0, volumeMAOutBegin = 0, atrOutBegin = 0,
      atrMAOutBegin = 0, macdOutBegin = 0, slowMAOutSize = 0, fastMAOutSize,
      volumeMAOutSize = 0, atrOutSize = 0, atrMAOutSize = 0, macdOutSize = 0,
      rsiOutBegin = 0, rsiOutSize = 0;
  const midas::InstrumentEnum instrument;
  std::vector<unsigned int> trades;
  std::vector<double> closePrices, volumes, highs, lows, opens, vwaps;
  std::vector<boost::posix_time::ptime> timestamps;
  unsigned int bullishCandles{0};

  void clearBuffers();

public:
  MomentumTrader(const std::shared_ptr<midas::DataStream> &source,
                 const std::shared_ptr<midas::OrderManager> &orderManager,
                 midas::InstrumentEnum instrument,
                 const std::shared_ptr<logging::thread_safe_logger_t> &logger);

  virtual ~MomentumTrader() = default;
  virtual void decide() override;
  virtual std::string traderName() const override;

protected:
  MomentumTrader(std::size_t bufferSize,
                 const std::shared_ptr<midas::DataStream> &source,
                 const std::shared_ptr<midas::OrderManager> &orderManager,
                 midas::InstrumentEnum instrument,
                 const std::shared_ptr<logging::thread_safe_logger_t> &logger);
  /**
   *
   * @param entryPrice decided entry price
   * @param direction are we going long or short
   * @return pair where first is profit limit and second is stop loss
   */
  virtual std::pair<double, double>
  decideProfitAndStopLossLevels(double entryPrice, OrderDirection direction);

  virtual std::size_t decideEntryQuantity();
  virtual void calculateTechnicalAnalysis();
};

class StockMomentumTrader : public MomentumTrader {

protected:
  std::size_t decideEntryQuantity() override;
  std::pair<double, double>
  decideProfitAndStopLossLevels(double entryPrice, OrderDirection direction) override;
};
} // namespace midas::trader