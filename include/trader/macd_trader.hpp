//
// Created by ahi on 1/6/25.
//
#include "trader/base_trader.hpp"

#ifndef MACD_TRADER_HPP
#define MACD_TRADER_HPP
namespace midas::trader {



class MacdTrader: public Trader {
  protected:
  std::recursive_mutex stateMutex;
  enum class TraderState {
    NoPosition,
    LongPosition,
    ShortPosition,
    Waiting,
  };
  static constexpr int fastMATimePeriod = 5;
  static constexpr int slowMATimePeriod = 15;
  static constexpr int macdFastPeriod = 6, macdSlowPeriod = 13,
                       macdSignalPeriod = 4;
  static constexpr int rsiTimePeriod = 6;
  InstrumentEnum instrument;

  std::array<double, 100> slowMa, fastMa, macd, macdSignal, macdHistogram, rsi;
  std::vector<double> closePrices, volumes, highs, lows, opens, vwaps;
  std::vector<boost::posix_time::ptime> timestamps;
  std::vector<unsigned int> trades;
  int slowMAOutBeg = 0, fastMAOutBeg = 0, macdOutBegin = 0, slowMAOutSize = 0, fastMAOutSize, macdOutSize = 0,
      rsiOutBegin = 0, rsiOutSize = 0;
  TraderState currentState{TraderState::NoPosition};
  const std::size_t entryQuantity;
  const bool useMKTOrders{true};
  const int numberOfConsecutivePeriodsRequired{3};
  std::optional<boost::posix_time::ptime> entryTime;

  void clearBuffers();


  public:
  MacdTrader(const std::shared_ptr<midas::DataStream> &source,
              const std::shared_ptr<midas::OrderManager> &orderManager,
              midas::InstrumentEnum instrument, std::size_t entryQuantity,
              const std::shared_ptr<logging::thread_safe_logger_t> &logger);
  virtual ~MacdTrader() = default;
  void decide() override;
  std::string traderName() const override;


protected:
  MacdTrader(std::size_t bufferSize,
              const std::shared_ptr<midas::DataStream> &source,
              const std::shared_ptr<midas::OrderManager> &orderManager,
              midas::InstrumentEnum instrument, std::size_t entryQuantity,
              const std::shared_ptr<logging::thread_safe_logger_t> &logger);
  virtual void calculateTechnicalAnalysis();
  void decideEntry();
  void decideLongExit();
  void decideShortExit();
  void executeOrder(OrderDirection direction, double quantity, std::function<void(Order::StatusChangeEvent)> callback);
};

}

#endif //MACD_TRADER_HPP
