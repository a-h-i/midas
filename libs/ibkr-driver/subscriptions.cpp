#include "CommonDefs.h"
#include "Contract.h"
#include "TagValue.h"
#include "broker-interface/subscription.hpp"
#include "ibkr-driver/ibkr.hpp"
#include "ibkr/internal/active_subscription_state.hpp"
#include "ibkr/internal/build_contracts.hpp"
#include "ibkr/internal/client.hpp"
#include "ibkr/internal/historical.hpp"
#include "logging/logging.hpp"
#include <functional>
#include <memory>
#include <mutex>

#include <stdexcept>
#include <tuple>
#include <utility>
#include <vector>

void ibkr::internal::Client::addSubscription(
    std::weak_ptr<midas::Subscription> subscription) {
  std::scoped_lock lock(subscriptionsMutex);
  pendingSubscriptions.push_back(subscription);
}

void ibkr::internal::Client::removeActiveSubscription(const TickerId ticker) {
  std::scoped_lock subscriptionManagementLock(subscriptionsMutex);
  activeSubscriptions.erase(ticker);
}

std::size_t ibkr::internal::Client::applyToActiveSubscriptions(
    std::function<bool(midas::Subscription &, ActiveSubscriptionState &state)>
        func,
    const TickerId ticker) {

  std::size_t numberProcessed = 0;
  std::scoped_lock subscriptionManagementLock(subscriptionsMutex);
  if (activeSubscriptions.contains(ticker)) {
    auto &subscriptionState = activeSubscriptions.at(ticker);
    std::shared_ptr<midas::Subscription> subscription =
        subscriptionState.subscription.lock();
    if (subscription) {
      const bool remove = func(*subscription, subscriptionState);
      if (remove) {
        activeSubscriptions.erase(ticker);
      }
      numberProcessed++;
    } else {
      activeSubscriptions.erase(ticker);
    }
  } else {
    // While a weak pointer may not have a reference anymore. It should still
    // exist in the map if this function is called
    ERROR_LOG(logger)
        << "Attempted to apply function to active subscriptions for ticker "
        << ticker << " but there are no associated subscriptions";
  }

  return numberProcessed;
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
    for (unsigned int i = 0; i < tickTypes.size(); i++) {
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
      auto insertedPair = activeSubscriptions.emplace(
          std::piecewise_construct, std::forward_as_tuple(tickerId),
          std::forward_as_tuple(weakPtr, tickerId));
      if (!insertedPair.second) {
        throw std::runtime_error(
            "Unable to insert entry into active subscriptions");
      }
      auto &activeSub = insertedPair.first->second;
      const Contract contract = build_contract(sub->symbol);
      if (sub->isRealtime) {
        std::vector<int> realTimeRequestIds = requestRealtimeData(
            contract, tickerId, connectionState.clientSocket.get(),
            sub->includeTickData);

      } else {
        BarSizeSetting barSizeSetting;
        requestHistoricalData(
            contract, tickerId, connectionState.clientSocket.get(),
            sub->historicalDuration.value_or(
                midas::HistorySubscriptionStartPoint{
                    .unit = midas::SubscriptionDurationUnits::Months,
                    .quantity = 1,
                }),
            barSizeSetting);
        activeSub.historicalBarSizeSetting = barSizeSetting;
      }
      // cancel slot is the same regardless if real time or historical
      activeSub.cancelConnection = sub->cancelSignal.connect(
          [this,
           tickerId]([[maybe_unused]] const midas::Subscription &subscription) {
            std::scoped_lock lock(commandsMutex);
            // queue command for processing in next process cycle
            pendingCommands.push_back(
                [this, tickerId]() { handleSubscriptionCancel(tickerId); });
          });
    }
  }
  pendingSubscriptions.clear();
}



void ibkr::internal::Client::handleSubscriptionCancel(const TickerId ticker) {
  std::scoped_lock lock(subscriptionsMutex);
  if (activeSubscriptions.contains(ticker)) {
    auto &state = activeSubscriptions.at(ticker);
    if (!state.isDone) {
      // we need to cancel pending api requests
      if (state.isRealTime) {
        connectionState.clientSocket->cancelHistoricalData(ticker);
      } else {
        for (int requestId : state.ticksByTickRequestIds) {
          connectionState.clientSocket->cancelTickByTickData(requestId);
        }
        connectionState.clientSocket->cancelRealTimeBars(ticker);
      }
    } else {
      state.isDone = true;
    }
    activeSubscriptions.erase(ticker);
  }
}

ibkr::internal::ActiveSubscriptionState::ActiveSubscriptionState(
    std::weak_ptr<midas::Subscription> subscription, TickerId ticker)
    : isDone(false), subscription(subscription), ticker(ticker) {
  auto shared = subscription.lock();
  if (!shared) {
    throw std::domain_error(
        "ActiveSubscriptionState initialized with ptr already destroyed");
  }
  isRealTime = shared->isRealtime;
}