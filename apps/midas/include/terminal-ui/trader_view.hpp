#pragma once

#include "broker-interface/trades_summary.hpp"
#include "terminal-ui/ui-component.hpp"
#include "trader/base_trader.hpp"
#include <boost/signals2/connection.hpp>
#include <ftxui/component/component_base.hpp>
#include <mutex>
#include <optional>
namespace ui {

/**
 * Represents the a view of a trader
 */
class TraderSummaryView : public UIComponenet {
  boost::signals2::scoped_connection tradeSummaryConnection;
  void refresh(midas::TradeSummary summary);
  std::optional<midas::TradeSummary> currentSummary;
  std::mutex summaryMutex;
  const std::string traderName;

public:
  TraderSummaryView(midas::trader::Trader &trader);
  virtual ftxui::Component render() const override;
};
} // namespace ui