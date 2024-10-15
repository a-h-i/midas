#include "ibkr/internal/historical.hpp"
#include "ibkr/internal/client.hpp"
#include "ibkr-driver/ibkr.hpp"

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
  }
  const auto durationStr = historicalDuration.str();
  barSize = historicalBarSize(start);
  socket->reqHistoricalData(ticker, contract, "", durationStr,
                            barSize.settingString, "TRADES", 0, 2, false,
                            TagValueListSPtr());
}

ibkr::internal::BarSizeSetting ibkr::internal::historicalBarSize(
    const midas::HistorySubscriptionStartPoint &start) {
  if (start.unit == midas::SubscriptionDurationUnits::Years ||
      start.quantity > 1) {
    return {.settingString = "1 min", .sizeSeconds = 60};
  } else {
    return {.settingString = "30 secs", .sizeSeconds = 30};
  }
}




void ibkr::internal::Client::historicalDataEnd(
    int ticker, [[maybe_unused]] const std::string &startDateStr,
    [[maybe_unused]] const std::string &endDateStr) {

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


unsigned int ibkr::Driver::estimateHistoricalBarSizeSeconds(
    const midas::HistorySubscriptionStartPoint &duration) const {
  return internal::historicalBarSize(duration).sizeSeconds;
}
