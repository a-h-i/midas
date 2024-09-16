#include "bar.h"
#include "ibkr/internal/bar_conversion.hpp"
#include "ibkr/internal/client.hpp"
#include "logging/logging.hpp"

void ibkr::internal::Client::tickPrice([[maybe_unused]] TickerId tickerId, TickType field,
                                       double price, const TickAttrib &attrib) {
  DEBUG_LOG(logger) << "TickPrice: " << field << " Price: " << price
                    << " attributes: " << attrib.canAutoExecute
                    << attrib.pastLimit << attrib.preOpen;
}

void ibkr::internal::Client::tickSize([[maybe_unused]] TickerId tickerId, TickType field,
                                      Decimal size) {
  DEBUG_LOG(logger) << "TickSize: " << size << " type: " << field << std::endl;
}

void ibkr::internal::Client::tickString([[maybe_unused]] TickerId tickerId, [[maybe_unused]] TickType tickType,
                                        const std::string &value) {
  DEBUG_LOG(logger) << "TICK STRING: " << value;
}

void ibkr::internal::Client::tickGeneric([[maybe_unused]] TickerId tickerId, [[maybe_unused]]TickType tickType,
                                         double value) {
  DEBUG_LOG(logger) << "TICK GENERIC: " << value;
}

void ibkr::internal::Client::tickEFP(TickerId tickerId, TickType tickType,
                                     double basisPoints,
                                     const std::string &formattedBasisPoints,
                                     double totalDividends, int holdDays,
                                     const std::string &futureLastTradeDate,
                                     double dividendImpact,
                                     double dividendsToLastTradeDate) {
  DEBUG_LOG(logger) << "TickEFP" << tickerId << ", Type: " << tickType
                    << ", BasisPoints: " << basisPoints
                    << ", FormattedBasisPoints: " << formattedBasisPoints
                    << ", Total Dividends: " << totalDividends
                    << ", HoldDays: " << holdDays
                    << ", Future Last Trade Date: " << futureLastTradeDate
                    << ", Dividend Impact: " << dividendImpact
                    << ", Dividends To Last Trade Date: "
                    << dividendsToLastTradeDate;
}

/**
 * Market Data Delayed
 * Delayed market data can only used with EClient.reqMktData and
EClient.reqHistoricalData. This does not function for tick data.

The API can request Live, Frozen, Delayed and Delayed Frozen
 market data from Trader Workstation by switching market data type via the
 EClient.reqMarketDataType before making a market data request. A successful
switch to a different (non-live) market data type for a particular market data
request will be indicated by a callback to EWrapper.marketDataType with the
 ticker ID of the market data request which is returning a different type of
data.
 */

void ibkr::internal::Client::historicalData(TickerId tickerId,
                                            const Bar &internalBar) {

  midas::Bar bar = ibkr::internal::convertIbkrBar(internalBar, 30);
  applyToActiveSubscriptions(
      [&bar](Subscription &subscription) {
        subscription.barListeners.notify(subscription, bar);
        return false;
      },
      tickerId);
}

/**
 * Real Time Data
 */

void ibkr::internal::Client::realtimeBar(TickerId tickerId, long time,
                                         double open, double high, double low,
                                         double close, Decimal volume,
                                         Decimal wap, int count) {
  Bar internalBar;
  internalBar.time = std::to_string(time);
  internalBar.high = high;
  internalBar.low = low;
  internalBar.open = open;
  internalBar.close = close;
  internalBar.wap = wap;
  internalBar.volume = volume;
  internalBar.count = count;
  midas::Bar bar = ibkr::internal::convertIbkrBar(internalBar, 30);
  applyToActiveSubscriptions(
      [&bar](Subscription &subscription) {
        subscription.barListeners.notify(subscription, bar);
        return false;
      },
      tickerId);
}

void ibkr::internal::Client::tickSnapshotEnd(int reqId) {
  DEBUG_LOG(logger) << "Tick snapshot end " << reqId;
}

void ibkr::internal::Client::marketDataType(TickerId reqId,
                                            int marketDataType) {
  DEBUG_LOG(logger) << "MarketDataType for ticker " << reqId << " is "
                    << marketDataType;
}

void ibkr::internal::Client::histogramData([[maybe_unused]] int reqId,
                                           [[maybe_unused]] const HistogramDataVector &data) {
  DEBUG_LOG(logger) << "Unsupported histogram feature";
}

void ibkr::internal::Client::tickByTickMidPoint(int reqId, time_t time,
                                                double midPoint) {
  DEBUG_LOG(logger) << "tickByTickMidpoint " << reqId << " at " << time
                    << " is " << midPoint;
}

void ibkr::internal::Client::tickByTickBidAsk(
    int reqId, time_t time, double bidPrice, double askPrice, Decimal bidSize,
    Decimal askSize, [[maybe_unused]] const TickAttribBidAsk &tickAttribBidAsk) {
  DEBUG_LOG(logger) << "tickByTickBidAsk " << reqId << " at " << time
                    << " bidPrice " << bidPrice << " askPrice " << askPrice
                    << " bidSize " << bidSize << " askSize " <<  askSize;
}

void ibkr::internal::Client::tickByTickAllLast(
    int reqId, int tickType, time_t time, double price, [[maybe_unused]] Decimal size,
    [[maybe_unused]] const TickAttribLast &tickAttribLast, const std::string &exchange,
    const std::string &specialConditions) {
  DEBUG_LOG(logger) << " tickByTickAllLast " << reqId << " type " << tickType
                    << " at " << time << " is " << price << " on exchange "
                    << exchange << " with conditions " << specialConditions;
}

void ibkr::internal::Client::historicalTicks(
    [[maybe_unused]] int reqId, [[maybe_unused]] const std::vector<HistoricalTick> &ticks,
    [[maybe_unused]] bool done) {
  DEBUG_LOG(logger) << "historical received";
}

void ibkr::internal::Client::historicalTicksBidAsk(
    [[maybe_unused]] int reqId, [[maybe_unused]] const std::vector<HistoricalTickBidAsk> &ticks,
    [[maybe_unused]] bool done) {
  DEBUG_LOG(logger) << "historical received";
}

void ibkr::internal::Client::historicalTicksLast(
    [[maybe_unused]] int reqId, [[maybe_unused]] const std::vector<HistoricalTickLast> &ticks,
    [[maybe_unused]] bool done) {
  DEBUG_LOG(logger) << "historical received";
}
