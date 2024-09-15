#include "ibkr/internal/client.hpp"

void ibkr::internal::Client::addSubscription(subscription_ptr_t subscription) {
  std::scoped_lock lock(subscriptionsMutex);
  pendingSubscriptions.push_back(subscription);
}

std::size_t ibkr::internal::Client::applyToActiveSubscriptions(
    std::function<bool (Subscription &)> func, const TickerId ticker) {

  std::size_t numberProcessed = 0;
  std::scoped_lock subscriptionManagementLock(subscriptionsMutex);
  if (activeSubscriptions.contains(ticker)) {
    std::shared_ptr<Subscription> subscription =
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
    ERROR_LOG(logger) << "Attempted to apply function to active subscriptions for ticker " << ticker
                      << " but there are no associated subscriptions";
  }

  return numberProcessed;
}