

#include "ibkr/internal/active_subscription_state.hpp"

ibkr::internal::ActiveSubscriptionState::~ActiveSubscriptionState() {
  if (cancelConnection.has_value()) {
    cancelConnection.value().disconnect();
  }
}