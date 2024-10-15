#pragma once

#include "EClientSocket.h"
#include "ibkr/internal/active_subscription_state.hpp"
namespace ibkr::internal {
  void
requestHistoricalData(const Contract &contract, const TickerId ticker,
                      EClientSocket *const socket,
                      const midas::HistorySubscriptionStartPoint &start,
                      BarSizeSetting &barSize);

                     BarSizeSetting
historicalBarSize(const midas::HistorySubscriptionStartPoint &start);

}