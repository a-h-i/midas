#include "Contract.h"
#include "ibkr/internal/client.hpp"
#include "logging/logging.hpp"

void ibkr::internal::Client::contractDetails(
    [[maybe_unused]] int reqId,
    [[maybe_unused]] const ContractDetails &contractDetails) {
  DEBUG_LOG(logger) << "Received contract details " << "Symbol "
                    << contractDetails.longName
                    << contractDetails.contract.multiplier;
}

void ibkr::internal::Client::contractDetailsEnd([[maybe_unused]] int reqId) {
  DEBUG_LOG(logger) << "contractDetailsEnd ";
}

void ibkr::internal::Client::securityDefinitionOptionalParameter(
    [[maybe_unused]] int reqId, [[maybe_unused]] const std::string &exchange,
    [[maybe_unused]] int underlyingConId,
    [[maybe_unused]] const std::string &tradingClass,
    [[maybe_unused]] const std::string &multiplier,
    [[maybe_unused]] const std::set<std::string> &expirations,
    [[maybe_unused]] const std::set<double> &strikes) {
  DEBUG_LOG(logger) << "Security Definition Optional Parameter. Request: "
                    << reqId << ", Trading Class: " << tradingClass
                    << ", Multiplier: " << multiplier;
}

void ibkr::internal::Client::securityDefinitionOptionalParameterEnd(
    [[maybe_unused]] int reqId) {
  DEBUG_LOG(logger) << "securityDefinitionOptionalParameterEnd";
}