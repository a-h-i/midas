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
  static constexpr int atrTimePeriod = 21;
  static constexpr int rsiTimePeriod = 6;

  static constexpr int macdFastPeriod = 6, macdSlowPeriod = 13,
                       macdSignalPeriod = 4;
  static constexpr int atrSmoothingPeriod = 9;
  static constexpr double commissionEstimatePerUnit = 0.25;
  static constexpr double roundingCoeff = 4; // minimum of .25 index moves
protected:
  std::atomic<int> bullishCandlesinARow{0}, bearishCandlesInARow{0};
  struct candle_decision_t {
    double bullishIndicator{-1}, bearishIndicator{-1}, maxBullish{0}, maxBearish{0};
  };
  std::array<double, 100> slowMa, fastMa, volumeMa, atr, atrMA, macd,
      macdSignal, macdHistogram, rsi, bbUpper, bbMiddle, bbLower;
  int slowMAOutBeg = 0, fastMAOutBeg = 0, volumeMAOutBegin = 0, atrOutBegin = 0,
      atrMAOutBegin = 0, macdOutBegin = 0, slowMAOutSize = 0, fastMAOutSize,
      volumeMAOutSize = 0, atrOutSize = 0, atrMAOutSize = 0, macdOutSize = 0,
      rsiOutBegin = 0, rsiOutSize = 0, bbBegIndex = 0, bbOutSize = 0;
  const midas::InstrumentEnum instrument;
  std::vector<unsigned int> trades;
  std::vector<double> closePrices, volumes, highs, lows, opens, vwaps;
  std::vector<boost::posix_time::ptime> timestamps;
  unsigned int bullishCandles{0};

  void clearBuffers();

public:
  MomentumTrader(const std::shared_ptr<midas::DataStream> &source,
                 const std::shared_ptr<midas::OrderManager> &orderManager,
                 midas::InstrumentEnum instrument, std::size_t entryQuantity,
                 const std::shared_ptr<logging::thread_safe_logger_t> &logger);

  virtual ~MomentumTrader() = default;
  void decide() override;
  std::string traderName() const override;

protected:
  MomentumTrader(std::size_t bufferSize,
                 const std::shared_ptr<midas::DataStream> &source,
                 const std::shared_ptr<midas::OrderManager> &orderManager,
                 midas::InstrumentEnum instrument, std::size_t entryQuantity,
                 const std::shared_ptr<logging::thread_safe_logger_t> &logger);
  const std::size_t entryQuantity;
  /**
   *
   * @param entryPrice decided entry price
   * @param direction are we going long or short
   * @return pair where first is profit limit and second is stop loss
   *
   */
  virtual std::pair<double, double>
  decideProfitAndStopLossLevels(double entryPrice, OrderDirection direction);
  /**
   *
   * @param candleEndOffset offset from the end of array. 0 implies last element
   * @return candle decision
   */
  virtual candle_decision_t decideCandle(std::size_t candleEndOffset);
  virtual std::size_t decideEntryQuantity();
  virtual double decideEntryPrice();
  virtual void calculateTechnicalAnalysis();
};

class StockMomentumTrader : public MomentumTrader {
public:
  StockMomentumTrader(
      const std::shared_ptr<midas::DataStream> &source,
      const std::shared_ptr<midas::OrderManager> &orderManager,
      midas::InstrumentEnum instrument, std::size_t entryQuantity,
      const std::shared_ptr<logging::thread_safe_logger_t> &logger)
      : MomentumTrader(100, source, orderManager, instrument, entryQuantity, logger) {}

protected:
  std::size_t decideEntryQuantity() override;
  std::pair<double, double>
  decideProfitAndStopLossLevels(double entryPrice,
                                OrderDirection direction) override;
  double decideEntryPrice() override;
};
} // namespace midas::trader