#pragma once
#include "order.hpp"
#include <logging/logging.hpp>

namespace midas {

struct TradeSummary {
  unsigned int numberOfEntryOrders{0}, numberOfStopLossTriggered{0},
      numberOfProfitTakersTriggered{0};
  double successRatio{0}, maxDownTurn{0}, maxUpTurn{0}, endingBalance{0};
  
};

class OrderSummaryTracker : public OrderVisitor {
  TradeSummary accumulator;


public:
  void visit(SimpleOrder &) override;
  void visit(BracketedOrder &) override;
  /**
    * Adds an order to the summary. Must be a filled order.
    * \throws logic error if order is not in filled state
   */
  void addToSummary(Order *order);
  inline TradeSummary summary() { return accumulator; };
  inline void clear() { accumulator = TradeSummary(); };
};
} // namespace midas