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