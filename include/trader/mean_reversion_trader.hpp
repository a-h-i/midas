//
// Created by potato on 28-12-2024.
//
# pragma once
#include "./base_trader.hpp"
namespace midas::trader {

class MeanReversionTrader: public Trader {

  protected:
  struct candle_decision_t {
    double bullishIndicator{-1}, bearishIndicator{-1}, maxBullish{0}, maxBearish{0};
  };
  static constexpr int roundingCoeff = 4;
  std::array<double, 100> bbUpper, bbMiddle, bbLower;
  int bbBegIndex = 0, bbOutSize = 0;
  std::vector<unsigned int> trades;
  std::vector<double> closePrices, volumes, highs, lows, opens, vwaps;
  std::vector<boost::posix_time::ptime> timestamps;
  const midas::InstrumentEnum instrument;
  const std::size_t entryQuantity;
  MeanReversionTrader( std::size_t bufferSize, const std::shared_ptr<midas::DataStream> &source,
                 const std::shared_ptr<midas::OrderManager> &orderManager,
                 midas::InstrumentEnum instrument, std::size_t entryQuantity,
                 const std::shared_ptr<logging::thread_safe_logger_t> &logger);
  void clearBuffers();
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

  public:
  MeanReversionTrader(const std::shared_ptr<midas::DataStream> &source,
               const std::shared_ptr<midas::OrderManager> &orderManager,
               midas::InstrumentEnum instrument, std::size_t entryQuantity,
               const std::shared_ptr<logging::thread_safe_logger_t> &logger);
    void decide() override;
    std::string traderName() const override;
    virtual ~MeanReversionTrader() = default;
};
}