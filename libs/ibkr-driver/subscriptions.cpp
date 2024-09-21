#include "Contract.h"
#include "TagValue.h"
#include "ibkr/internal/build_contracts.hpp"
#include "ibkr/internal/client.hpp"
#include <vector>

void ibkr::internal::Client::addSubscription(
    std::weak_ptr<midas::Subscription> subscription) {
  std::scoped_lock lock(subscriptionsMutex);
  pendingSubscriptions.push_back(subscription);
}

std::size_t ibkr::internal::Client::applyToActiveSubscriptions(
    std::function<bool(midas::Subscription &)> func, const TickerId ticker) {

  std::size_t numberProcessed = 0;
  std::scoped_lock subscriptionManagementLock(subscriptionsMutex);
  if (activeSubscriptions.contains(ticker)) {
    std::shared_ptr<midas::Subscription> subscription =
        activeSubscriptions.at(ticker).lock();
    if (subscription) {
      const bool remove = func(*subscription);
      if (remove) {
        activeSubscriptions.erase(ticker);
      }
      numberProcessed++;
    } else {
      activeSubscriptions.erase(ticker);
    }
  } else {
    ERROR_LOG(logger)
        << "Attempted to apply function to active subscriptions for ticker "
        << ticker << " but there are no associated subscriptions";
  }

  return numberProcessed;
}

static void requestHistoricalData(const Contract &contract,
                                  const TickerId ticker,
                                  EClientSocket *const socket) {
  socket->reqHistoricalData(ticker, contract, "", "1 W", "30 secs", "TRADES", 0,
                            2, false, TagValueListSPtr());
}

static std::vector<int> requestRealtimeData(const Contract &contract,
                                            TickerId tickerId,
                                            EClientSocket *socket,
                                            bool includeTickData) {

  static const std::array<std::string, 4> tickTypes{"Last", "MidPoint",
                                                    "BidAsk", "AllLast"};
  std::vector<int> requestIds;
  const int requestOffset = tickerId << 8;
  if (includeTickData) {
    for (int i = 0; i < tickTypes.size(); i++) {
      const int reqId = requestOffset + i;
      requestIds.push_back(reqId);
      socket->reqTickByTickData(reqId, contract, tickTypes[i], 0, false);
    }
  }
  socket->reqRealTimeBars(tickerId, contract, 5, "TRADES", false,
                          TagValueListSPtr());
  return requestIds;
}

void ibkr::internal::Client::processPendingSubscriptions() {

  std::scoped_lock lock(subscriptionsMutex);
  for (auto weakPtr : pendingSubscriptions) {
    std::shared_ptr<midas::Subscription> sub = weakPtr.lock();
    if (sub) {
      const TickerId tickerId = nextTickerId++;
      activeSubscriptions[tickerId] = weakPtr;
      const Contract contract = build_futures_contract(sub->symbol);
      if (sub->isRealtime) {
        std::vector<int> realTimeRequestIds = requestRealtimeData(
            contract, tickerId, connectionState.clientSocket.get(),
            sub->includeTickData);
        sub->cancelListeners.add_listener(
            [this, tickerId,
             realTimeRequestIds]([[maybe_unused]] const midas::Subscription &) {
              for (auto requestId : realTimeRequestIds) {
                connectionState.clientSocket->cancelTickByTickData(requestId);
              }
              connectionState.clientSocket->cancelRealTimeBars(tickerId);
            });
      } else {
        requestHistoricalData(contract, tickerId,
                              connectionState.clientSocket.get());
        sub->cancelListeners.add_listener(
            [this, tickerId]([[maybe_unused]] const midas::Subscription &) {
              connectionState.clientSocket->cancelHistoricalData(tickerId);
            });
      }
    }
  }
  pendingSubscriptions.clear();
}

void ibkr::internal::Client::historicalDataEnd(
    int ticker, [[maybe_unused]] const std::string &startDateStr,
    [[maybe_unused]] const std::string &endDateStr) {
  applyToActiveSubscriptions(
      [](midas::Subscription &subscription) {
        subscription.endListeners.notify(subscription);
        return true;
      },
      ticker);
}