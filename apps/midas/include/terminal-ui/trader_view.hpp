#pragma once

#include "broker-interface/trades_summary.hpp"
#include "trader/trader.hpp"
namespace ui {

/**
 * Represents the a view of a trader
 */
class TraderView {

public:
  TraderView(midas::trader::Trader &trader);
  ~TraderView();

  void refresh(midas::TradeSummary);

private:
  boost::signals2::connection traderSignalConnection;
};
} // namespace ui