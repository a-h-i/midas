#include "Contract.h"
#include "ibkr/internal/client.hpp"
#include "logging/logging.hpp"

void ibkr::internal::Client::contractDetails(
    int reqId, const ContractDetails &contractDetails) {
  DEBUG_LOG(logger) << "Received contract details " << "Symbol "
                    << contractDetails.longName << contractDetails.contract.multiplier;
}

void ibkr::internal::Client::contractDetailsEnd(int reqId) {
  DEBUG_LOG(logger) << "contractDetailsEnd ";
}

void ibkr::internal::Client::securityDefinitionOptionalParameter(
    int reqId, const std::string &exchange, int underlyingConId,
    const std::string &tradingClass, const std::string &multiplier,
    const std::set<std::string> &expirations, const std::set<double> &strikes) {
  DEBUG_LOG(logger) << "Security Definition Optional Parameter. Request: "
                    << reqId << ", Trading Class: " << tradingClass
                    << ", Multiplier: " << multiplier;
}

void ibkr::internal::Client::securityDefinitionOptionalParameterEnd(int reqId) {
  DEBUG_LOG(logger) << "securityDefinitionOptionalParameterEnd";
}