#pragma once

namespace midas {
struct TradeSummary {
  unsigned int numberOfEntryOrders{0}, numberOfStopLossTriggered{0},
      numberOfProfitTakersTriggered{0};
  double successRatio{0}, maxDownTurn{0}, maxUpTurn{0}, endingBalance{0};
};

} // namespace midas