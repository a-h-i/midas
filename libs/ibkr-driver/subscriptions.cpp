#include "CommonDefs.h"
#include "Contract.h"
#include "TagValue.h"
#include "broker-interface/subscription.hpp"
#include "ibkr-driver/ibkr.hpp"
#include "ibkr/internal/active_subscription_state.hpp"
#include "ibkr/internal/build_contracts.hpp"
#include "ibkr/internal/client.hpp"
#include <functional>
#include <memory>
#include <sstream>
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

static ibkr::internal::BarSizeSetting
historicalBarSize(const midas::HistorySubscriptionStartPoint &start) {
  if (start.unit == midas::SubscriptionDurationUnits::Years ||
      start.quantity > 1) {
    return {.settingString = "1 min", .sizeSeconds = 60};
  } else {
    return {.settingString = "30 secs", .sizeSeconds = 30};
  }
}

unsigned int ibkr::Driver::estimateHistoricalBarSizeSeconds(
    const midas::HistorySubscriptionStartPoint &duration) const {
  return historicalBarSize(duration).sizeSeconds;
}

static void
requestHistoricalData(const Contract &contract, const TickerId ticker,
                      EClientSocket *const socket,
                      const midas::HistorySubscriptionStartPoint &start,
                      ibkr::internal::BarSizeSetting &barSize) {
  std::stringstream historicalDuration;
  historicalDuration << start.quantity;
  switch (start.unit) {

  case midas::SubscriptionDurationUnits::Years:
    historicalDuration << " Y";
    break;
  case midas::SubscriptionDurationUnits::Months:
    historicalDuration << " M";
    break;
  }
  const auto durationStr = historicalDuration.str();
  barSize = historicalBarSize(start);
  socket->reqHistoricalData(ticker, contract, "", durationStr,
                            barSize.settingString, "TRADES", 0, 2, false,
                            TagValueListSPtr());
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
      auto &activeSub = activeSubscriptions[tickerId] = {
          .isDone = false,
          .subscription = weakPtr,
          .ticker = tickerId,
      };
      const Contract contract = build_futures_contract(sub->symbol);
      if (sub->isRealtime) {
        std::vector<int> realTimeRequestIds = requestRealtimeData(
            contract, tickerId, connectionState.clientSocket.get(),
            sub->includeTickData);
        activeSub.cancelConnection = sub->cancelSignal.connect(
            std::bind(&Client::handleSubscriptionCancel, this, tickerId,
                      std::placeholders::_1));
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
        activeSub.cancelConnection = sub->cancelSignal.connect(
            std::bind(&Client::handleSubscriptionCancel, this, tickerId,
                      std::placeholders::_1));
      }
    }
  }
  pendingSubscriptions.clear();
}

void ibkr::internal::Client::historicalDataEnd(
    int ticker, [[maybe_unused]] const std::string &startDateStr,
    [[maybe_unused]] const std::string &endDateStr) {
  {
    std::scoped_lock lock(subscriptionsMutex);
    if (activeSubscriptions.contains(ticker)) {
      auto &activeSub = activeSubscriptions[ticker];
      activeSub.isDone = true;
      if (activeSub.cancelConnection.has_value()) {
        activeSub.cancelConnection.value().disconnect();
      }
    }
  }
  applyToActiveSubscriptions(
      [](midas::Subscription &subscription,
         [[maybe_unused]] ActiveSubscriptionState &state) {
        subscription.endSignal(subscription);
        return true;
      },
      ticker);
}

void ibkr::internal::Client::handleSubscriptionCancel(
    const TickerId ticker, const midas::Subscription &sub) {
  std::scoped_lock lock(subscriptionsMutex);
  if (activeSubscriptions.contains(ticker)) {
    auto &state = activeSubscriptions[ticker];
    if (!state.isDone) {
      // we need to cancel pending api requests
      if (sub.isRealtime) {
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