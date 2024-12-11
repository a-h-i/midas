#include "broker-interface/subscription.hpp"
#include "ibkr-driver/ibkr.hpp"
#include "ibkr/internal/client.hpp"
#include "ibkr/internal/historical.hpp"
#include "logging/logging.hpp"

void ibkr::internal::requestHistoricalData(
    const Contract &contract, const TickerId ticker,
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
  case midas::SubscriptionDurationUnits::Seconds:
    historicalDuration << " S";
  }
  const auto durationStr = historicalDuration.str();
  barSize = historicalBarSize(start);
  socket->reqHistoricalData(ticker, contract, "", durationStr,
                            barSize.settingString, "TRADES", 0, 2, false,
                            TagValueListSPtr());
}

ibkr::internal::BarSizeSetting ibkr::internal::historicalBarSize(
    const midas::HistorySubscriptionStartPoint &start) {
  if (start.unit == midas::SubscriptionDurationUnits::Years) {
    return {.settingString = "1 min", .sizeSeconds = 60};
  } else if (start.unit == midas::SubscriptionDurationUnits::Seconds) {
    return {.settingString = "5 secs", .sizeSeconds = 5};
  } else {
    return {.settingString = "30 secs", .sizeSeconds = 30};
  }
}

void ibkr::internal::Client::historicalDataEnd(
    int ticker, [[maybe_unused]] const std::string &startDateStr,
    [[maybe_unused]] const std::string &endDateStr) {
  INFO_LOG(logger) << "received historical data end event";
  // We notify clients that the historical data has ended.
  applyToActiveSubscriptions(
      [](midas::Subscription &subscription, ActiveSubscriptionState &state) {
        state.isDone = true;
        if (state.cancelConnection.has_value()) {
          state.cancelConnection.value().disconnect();
        }
        subscription.endSignal(subscription);
        return true;
      },
      ticker);
}
