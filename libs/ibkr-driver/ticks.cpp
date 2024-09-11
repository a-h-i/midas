#include "bar.h"
#include "ibkr/internal/client.hpp"
#include "logging/logging.hpp"

void ibkr::internal::Client::tickPrice(TickerId tickerId, TickType field,
                                       double price, const TickAttrib &attrib) {
  DEBUG_LOG(logger) << "TickPrice: " << field << " Price: " << price
                    << " attributes: " << attrib.canAutoExecute
                    << attrib.pastLimit << attrib.preOpen;
}

void ibkr::internal::Client::tickSize(TickerId tickerId, TickType field,
                                      Decimal size) {
  DEBUG_LOG(logger) << "TickSize: " << size << " type: " << field << std::endl;
}

void ibkr::internal::Client::tickString(TickerId tickerId, TickType tickType,
                                        const std::string &value) {
  DEBUG_LOG(logger) << "TICK STRING: " << value;
}

void ibkr::internal::Client::tickGeneric(TickerId tickerId, TickType tickType,
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

void ibkr::internal::Client::historicalData(TickerId reqId, const Bar &bar) {
  DEBUG_LOG(logger) << "Received Historical candle tickerId: " << reqId
                    << " high: " << bar.high << " low: " << bar.low
                    << " Open: " << bar.open << " close: " << bar.close
                    << " wap: " << bar.wap << " volume " << bar.volume
                    << " count: " << bar.count << " time " << bar.time;
}
void ibkr::internal::Client::historicalDataEnd(int reqId,
                                               const std::string &startDateStr,
                                               const std::string &endDateStr) {
  DEBUG_LOG(logger) << "Received historical data end " << reqId << " start at "
                    << startDateStr << " end at " << endDateStr;
}

/**
 * Real Time Data
 */

void ibkr::internal::Client::realtimeBar(TickerId reqId, long time, double open,
                                         double high, double low, double close,
                                         Decimal volume, Decimal wap,
                                         int count) {
  DEBUG_LOG(logger) << "Received realtime bar" << reqId << " time " << time
                    << " open " << open << " high " << high << " low " << low
                    << " close " << close << " volume " << volume << " wap "
                    << wap << " count " << count;
}

void ibkr::internal::Client::tickSnapshotEnd(int reqId) {
  DEBUG_LOG(logger) << "Tick snapshot end " << reqId;
}

void ibkr::internal::Client::marketDataType(TickerId reqId,
                                            int marketDataType) {
  DEBUG_LOG(logger) << "MarketDataType for ticker " << reqId << " is "
                    << marketDataType;
}

void ibkr::internal::Client::histogramData(int reqId,
                                           const HistogramDataVector &data) {
  DEBUG_LOG(logger) << "Unsupported histogram feature";
}

void ibkr::internal::Client::tickByTickMidPoint(int reqId, time_t time,
                                                double midPoint) {
  DEBUG_LOG(logger) << "tickByTickMidpoint " << reqId << " at " << time
                    << " is " << midPoint;
}

void ibkr::internal::Client::tickByTickBidAsk(
    int reqId, time_t time, double bidPrice, double askPrice, Decimal bidSize,
    Decimal askSize, const TickAttribBidAsk &tickAttribBidAsk) {
  DEBUG_LOG(logger) << "tickByTickBidAsk " << reqId << " at " << time
                    << " bidPrice " << bidPrice << " askPrice " << askPrice
                    << " bidSize " << bidSize << " askSize " << " askSize";
}

void ibkr::internal::Client::tickByTickAllLast(
    int reqId, int tickType, time_t time, double price, Decimal size,
    const TickAttribLast &tickAttribLast, const std::string &exchange,
    const std::string &specialConditions) {
  DEBUG_LOG(logger) << " tickByTickAllLast " << reqId << " type " << tickType
                    << " at " << time << " is " << price << " on exchange "
                    << exchange << " with conditions " << conditions;
}

void ibkr::internal::Client::historicalTicks(
    int reqId, const std::vector<HistoricalTick> &ticks, bool done) {
  DEBUG_LOG(logger) << "historical received";
}

void ibkr::internal::Client::historicalTicksBidAsk(
    int reqId, const std::vector<HistoricalTickBidAsk> &ticks, bool done) {
  DEBUG_LOG(logger) << "historical received";
}

void ibkr::internal::Client::historicalTicksLast(
    int reqId, const std::vector<HistoricalTickLast> &ticks, bool done) {
  DEBUG_LOG(logger) << "historical received";
}
