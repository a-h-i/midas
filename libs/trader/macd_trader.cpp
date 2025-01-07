//
// Created by ahi on 1/6/25.
//
#include "trader/macd_trader.hpp"

#include <ta_func.h>
using namespace midas;
using namespace midas::trader;

MacdTrader::MacdTrader(
    const std::shared_ptr<midas::DataStream> &source,
    const std::shared_ptr<midas::OrderManager> &orderManager,
    midas::InstrumentEnum instrument, std::size_t entryQuantity,
    const std::shared_ptr<logging::thread_safe_logger_t> &logger)
    : MacdTrader(100, source, orderManager, instrument, entryQuantity, logger) {

}

MacdTrader::MacdTrader(
    std::size_t bufferSize, const std::shared_ptr<midas::DataStream> &source,
    const std::shared_ptr<midas::OrderManager> &orderManager,
    midas::InstrumentEnum instrument, std::size_t entryQuantity,
    const std::shared_ptr<logging::thread_safe_logger_t> &logger)
    : Trader(bufferSize, 120, source, orderManager, logger),
      instrument(instrument), entryQuantity(entryQuantity) {
  closePrices.reserve(data.lookBackSize);
  volumes.reserve(data.lookBackSize);
  highs.reserve(data.lookBackSize);
  lows.reserve(data.lookBackSize);
  opens.reserve(data.lookBackSize);
  trades.reserve(data.lookBackSize);
  timestamps.reserve(data.lookBackSize);
}

void MacdTrader::calculateTechnicalAnalysis() {
  clearBuffers();
  data.copy(trades, highs, lows, opens, closePrices, vwaps, volumes,
            timestamps);
  TA_MACD(0, closePrices.size() - 1, closePrices.data(), macdFastPeriod,
          macdSlowPeriod, macdSignalPeriod, &macdOutBegin, &macdOutSize,
          macd.data(), macdSignal.data(), macdHistogram.data());
  TA_RSI(0, closePrices.size() - 1, closePrices.data(), rsiTimePeriod,
         &rsiOutBegin, &rsiOutSize, rsi.data());
  TA_EMA(0, closePrices.size() - 1, closePrices.data(), fastMATimePeriod,
         &fastMAOutBeg, &fastMAOutSize, fastMa.data());
  TA_EMA(0, closePrices.size() - 1, closePrices.data(), slowMATimePeriod,
         &slowMAOutBeg, &slowMAOutSize, slowMa.data());
}

void MacdTrader::clearBuffers() {
  closePrices.clear();
  volumes.clear();
  highs.clear();
  highs.clear();
  lows.clear();
  opens.clear();
  trades.clear();
  timestamps.clear();
}

std::string MacdTrader::traderName() const {
  return std::string("MACD trader - ") + instrument;
}

void MacdTrader::decide() {
  if (!data.ok() || paused()) {
    std::string statusString;
    if (paused()) {
      statusString = "Paused";
    } else {
      statusString = "Not enough data";
    }
    Trader::decision_params_t decisionParams{{statusString, "true"}};
    decisionParamsSignal(decisionParams);
    return;
  }
  calculateTechnicalAnalysis();

  switch (currentState) {
  case TraderState::NoPosition:
    decideEntry();
    break;
  case TraderState::LongPosition:
    decideLongExit();
    break;
  case TraderState::ShortPosition:
    decideShortExit();
    break;
  case TraderState::Waiting:
    break;
  }
}

void MacdTrader::decideLongExit() {
  // We exit our long position when the macd histogram starts declining
  // or macd MA drops below signal or we are in overbought territory
  if (currentState != TraderState::LongPosition) {
    return;
  }
  bool overbought = rsi[rsiOutSize - 1] > 80;
  bool macdCrossing = macd[macdOutSize - 1] < macdSignal[macdOutSize - 1];
  bool histogramDeclining = std::abs(macdHistogram[macdOutSize - 1]) <
                            std::abs(macdHistogram[macdOutSize - 2]);
  if (overbought || macdCrossing || histogramDeclining) {
    currentState = TraderState::Waiting;
    executeMarket(instrument, entryQuantity, OrderDirection::SELL,
                  [this](Order::StatusChangeEvent event) {
                    if (event.newStatus == OrderStatusEnum::Filled ||
                        event.newStatus == OrderStatusEnum::Cancelled) {
                      this->currentState = TraderState::NoPosition;
                    }
                  });
  }
}

void MacdTrader::decideShortExit() {
  if (currentState != TraderState::ShortPosition) {
    return;
  }
  bool oversold = rsi[rsiOutSize - 1] < 25;
  bool macdCrossing = macd[macdOutSize - 1] > macdSignal[macdOutSize - 1];
  bool histogramDeclining = std::abs(macdHistogram[macdOutSize - 1]) <
                            std::abs(macdHistogram[macdOutSize - 2]);
  if (oversold || macdCrossing || histogramDeclining) {
    currentState = TraderState::Waiting;
    executeMarket(instrument, entryQuantity, OrderDirection::BUY,
                  [this](Order::StatusChangeEvent event) {
                    if (event.newStatus == OrderStatusEnum::Filled ||
                        event.newStatus == OrderStatusEnum::Cancelled) {
                      this->currentState = TraderState::NoPosition;
                    }
                  });
  }
}

void MacdTrader::decideEntry() {
  if (currentState != TraderState::NoPosition) {
    return;
  }
  // We want to enter near when the macd crosses over the signal.
  // for longs we are looking for change in histogram from negative to positive
  // for shorts we are looking for change from positive to negative

  const std::size_t maxCrossOverDistance = 3;
  auto lastNegativeHistogramItr =
      std::find_if(macdHistogram.rbegin(), macdHistogram.rend(),
                   [](double x) { return x < 0; });
  std::size_t lastNegativeHistogramEndDistance =
      std::distance(macdHistogram.rbegin(), lastNegativeHistogramItr);
  bool macdCrossPositive =
      lastNegativeHistogramEndDistance > 1 &&
      lastNegativeHistogramEndDistance <= maxCrossOverDistance;

  auto lastPositiveHistogramItr =
      std::find_if(macdHistogram.rbegin(), macdHistogram.rend(),
                   [](double x) { return x > 0; });
  std::size_t lastPositiveHistogramEndDistance =
      std::distance(macdHistogram.rbegin(), lastPositiveHistogramItr);
  bool macdCrossNegative =
      lastPositiveHistogramEndDistance > 1 &&
      lastPositiveHistogramEndDistance <= maxCrossOverDistance;

  auto executeCallback =
      [this](Order::StatusChangeEvent event, TraderState targetState) {
        if (event.newStatus == OrderStatusEnum::Filled) {
          currentState = targetState;
        } else if (event.newStatus == OrderStatusEnum::Cancelled) {
          currentState = TraderState::NoPosition;
        }
      }

  if (macdCrossPositive) {
    currentState = TraderState::Waiting;
    executeMarket(instrument, entryQuantity, OrderDirection::BUY,
                  std::bind(executeCallback, std::placeholders::_1,
                            TraderState::LongPosition));
  }
  else if (macdCrossNegative) {
    currentState = TraderState::Waiting;
    executeMarket(instrument, entryQuantity, OrderDirection::SELL,
                  std::bind(executeCallback, std::placeholders::_1,
                            TraderState::ShortPosition));
  }
}