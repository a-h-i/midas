#pragma once

#include "broker-interface/trades_summary.hpp"
#include "terminal-ui/decision_params_view.hpp"
#include "terminal-ui/ui-component.hpp"
#include "trader/base_trader.hpp"
#include <boost/signals2/connection.hpp>
#include <chrono>
#include <ftxui/component/component_base.hpp>
#include <mutex>
#include <optional>
namespace ui {

/**
 * Represents the a view of a trader
 */
class TraderSummaryView : public UIComponenet {
  boost::signals2::scoped_connection tradeSummaryConnection;
  DecisionParamsView paramsView;
  std::optional<midas::TradeSummary> currentSummary;
  std::recursive_mutex summaryMutex;
  const std::string traderName;
  std::optional<std::chrono::seconds> updatedAt, lastPaintedAt;

  void refresh(midas::TradeSummary summary);

public:
  TraderSummaryView(midas::trader::Trader &trader);
  virtual bool dirty() const override;
protected:
  virtual ftxui::Component paint() override;
  
};
} // namespace ui