#pragma once
#include "CommonDefs.h"
#include "broker-interface/subscription.hpp"
#include <boost/signals2/connection.hpp>
#include <memory>
namespace ibkr::internal {
struct BarSizeSetting {
  std::string settingString;
  unsigned int sizeSeconds;
};

struct ActiveSubscriptionState {
  bool isDone, isRealTime; 
  std::weak_ptr<midas::Subscription> subscription;
  TickerId ticker;
  std::optional<boost::signals2::connection> cancelConnection;
  std::list<int> ticksByTickRequestIds;
  std::optional<BarSizeSetting> historicalBarSizeSetting;

  ActiveSubscriptionState(std::weak_ptr<midas::Subscription> subscription, TickerId ticker);
  ~ActiveSubscriptionState();
};
} // namespace ibkr::internal