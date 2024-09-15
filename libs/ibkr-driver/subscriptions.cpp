#include "ibkr/internal/client.hpp"


void ibkr::internal::Client::addSubscription(subscription_ptr_t subscription) {
  std::scoped_lock lock(subscriptionsMutex);
  pendingSubscriptions.push_back(subscription);
}
