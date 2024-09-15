#include "ibkr/internal/client.hpp"
#include "logging/logging.hpp"
#include <ranges>
#include <string_view>

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

void ibkr::internal::Client::accountDownloadEnd(
    const std::string &accountName) {
  DEBUG_LOG(logger) << "AccountDownloadEnd " << accountName;
}

void ibkr::internal::Client::managedAccounts(const std::string &accountsList) {
  auto accounts = std::views::split(accountsList, ",");
  for (const auto account : accounts) {
    managedAccountIds.emplace_back(std::string_view(account));
  }
  connectionState.notifyManagedAccountsReceived();
}

void ibkr::internal::Client::position(const std::string &account,
                                      const Contract &contract,
                                      Decimal position, double avgCost) {
  ERROR_LOG(logger)
      << "Unsupported event, driver does not track account positions atm";
}

void ibkr::internal::Client::positionEnd() {
  ERROR_LOG(logger)
      << "Unsupported event, driver does not track account positions atm";
}

void ibkr::internal::Client::accountSummary(int reqId,
                                            const std::string &account,
                                            const std::string &tag,
                                            const std::string &value,
                                            const std::string &currency) {
  DEBUG_LOG(logger) << "Account summary received reqId: " << reqId
                    << " account: " << account << " value: " << value
                    << " currency " << currency;
}

void ibkr::internal::Client::accountSummaryEnd(int reqId) {
  DEBUG_LOG(logger) << "Received account summary end: " << reqId;
}

void ibkr::internal::Client::pnlSingle(int reqId, Decimal pos, double dailyPnL,
                                       double unrealizedPnL, double realizedPnL,
                                       double value) {

  DEBUG_LOG(logger) << "Received pnl " << reqId << " pos " << pos
                    << " Daily pnl " << dailyPnL << " unrealized pnl "
                    << unrealizedPnL << " realized " << realizedPnL << " value "
                    << value;
}

void ibkr::internal::Client::pnl(int reqId, double dailyPnL,
                                 double unrealizedPnL, double realizedPnL) {
  DEBUG_LOG(logger) << "Received pnl " << reqId << " Daily pnl " << dailyPnL
                    << " unrealized pnl " << unrealizedPnL << " realized "
                    << realizedPnL;
}