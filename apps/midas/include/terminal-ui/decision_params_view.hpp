#pragma once

#include "terminal-ui/ui-component.hpp"
#include "trader/base_trader.hpp"
#include <boost/signals2/connection.hpp>
#include <mutex>
namespace ui {

class DecisionParamsView : public UIComponenet {
  boost::signals2::scoped_connection decisionParamsConnection;
  std::optional<midas::trader::Trader::decision_params_t> currentParams;
  std::mutex paramsMutex;
  std::optional<std::chrono::seconds> updatedAt, lastPaintedAt;
  void refresh(midas::trader::Trader::decision_params_t &);

public:
  DecisionParamsView(midas::trader::Trader &trader);
  virtual bool dirty() const override;
protected:
  virtual ftxui::Component paint() override;
  
};

} // namespace ui