#include "terminal-ui/decision_params_view.hpp"
#include <boost/bind/placeholders.hpp>
#include <ftxui/component/component.hpp>
#include <ftxui/component/component_base.hpp>
#include <mutex>
#include <sstream>
#include <vector>

using midas::trader::Trader;
using namespace ftxui;
using namespace ui;

DecisionParamsView::DecisionParamsView(Trader &trader)
    : decisionParamsConnection(trader.connectDecisionParamsSlot(
          Trader::decision_params_signal_t::slot_type(
              &DecisionParamsView::refresh, this, boost::placeholders::_1))) {}

void DecisionParamsView::refresh(Trader::decision_params_t &params) {
  std::scoped_lock paramsLock(paramsMutex);
  currentParams = params;
  updatedAt = std::chrono::duration_cast<std::chrono::seconds>(
      std::chrono::utc_clock::now().time_since_epoch());
}

Component DecisionParamsView::paint() {
  Element document;
  if (!currentParams.has_value()) {
    document = text("No decision params received");
  } else {
    std::vector<Element> elements;
    std::stringstream formatter;
    auto params = currentParams.value(); 
    for(const auto &param:  params) {
      formatter.imbue(std::locale("en_US"));
      formatter << std::boolalpha;

      formatter << param.first << " : " << param.second;
      elements.push_back(text(formatter.str()));
      formatter = std::stringstream();
    }
    document = vbox(elements);
  }

  auto renderer = Renderer([document] { return  window(text("Decision Params"), document);; });
  lastPaintedAt = std::chrono::duration_cast<std::chrono::seconds>(
      std::chrono::utc_clock::now().time_since_epoch());
  return renderer;
}

bool DecisionParamsView::dirty() const {
  if (!lastPaintedAt.has_value() || !updatedAt.has_value()) {
    return true;
  }
  return lastPaintedAt <= updatedAt;
}