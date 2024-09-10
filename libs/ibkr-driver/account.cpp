#include "ibkr/internal/client.hpp"
#include "logging/logging.hpp"

void ibkr::internal::Client::updateAccountValue(
    const std::string &key, const std::string &val, const std::string &currency,
    const std::string &accountName) {
  DEBUG_LOG(logger) << "UpdateAccountValue: key: " << key << " value: " << val
                    << " currency: " << currency
                    << " accountName: " << accountName;
}

void ibkr::internal::Client::updatePortfolio(
    const Contract &contract, Decimal position, double marketPrice,
    double marketValue, double averageCost, double unrealizedPNL,
    double realizedPNL, const std::string &accountName) {
  DEBUG_LOG(logger) << "UpdatePortfolio: contract: " << contract.symbol
                    << " Position: " << position << " unrealized "
                    << unrealizedPNL << " realized " << realizedPNL;
}

void ibkr::internal::Client::updateAccountTime(const std::string &timestamp) {
  DEBUG_LOG(logger) << "UpdateAccountTime " << timestamp;
}

 void ibkr::internal::Client::accountDownloadEnd(const std::string &accountName) {
  DEBUG_LOG(logger) << "AccountDownloadEnd " << accountName;
 }

 
 void ibkr::internal::Client::managedAccounts(const std::string &accountsList) {
  DEBUG_LOG(logger) << "Received managed accounts list " << accountsList;
 }