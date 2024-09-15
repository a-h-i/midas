#include "Contract.h"
#include "TagValue.h"
#include "ibkr/internal/build_contracts.hpp"
#include "ibkr/internal/client.hpp"

static void requestHistoricalData(const Contract &contract,
                                  const TickerId ticker,
                                  EClientSocket *const socket) {
  socket->reqHistoricalData(ticker, contract, "", "1 W", "30 secs", "TRADES", 0,
                            2, false, TagValueListSPtr());
}

void ibkr::internal::Client::processPendingSubscriptions() {

  std::scoped_lock lock(subscriptionsMutex);
  for (auto weakPtr : pendingSubscriptions) {
    std::shared_ptr<Subscription> sub = weakPtr.lock();
    if (sub) {
      const TickerId tickerId = nextTickerId++;
      activeSubscriptions[tickerId] = weakPtr;
      const Contract contract = build_futures_contract(sub->symbol);
      if (sub->isRealtime) {

      } else {
        requestHistoricalData(contract, tickerId,
                              connectionState.clientSocket.get());
      }
    }
  }
  pendingSubscriptions.clear();
}

void ibkr::internal::Client::historicalDataEnd(
    int ticker, [[maybe_unused]] const std::string &startDateStr,
    [[maybe_unused]] const std::string &endDateStr) {

  applyToActiveSubscriptions(
      [](Subscription &subscription) {
        subscription.endListeners.notify(subscription);
        return true;
      },
      ticker);
  std::scoped_lock lock(subscriptionsMutex);
}