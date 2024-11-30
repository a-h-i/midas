#include "terminal-ui/trader_view.hpp"
#include "trader/base_trader.hpp"
#include <boost/bind/placeholders.hpp>
#include <chrono>
#include <ftxui/component/component.hpp>
#include <ftxui/component/component_base.hpp>
#include <ftxui/dom/elements.hpp>
#include <iomanip>
#include <ios>
#include <locale>
#include <mutex>
#include <sstream>
#include <string>
#include <vector>

using midas::trader::Trader;
using namespace ftxui;

ui::TraderSummaryView::TraderSummaryView(Trader &trader)
    : tradeSummaryConnection(
          trader.connectSlot(Trader::trade_summary_signal_t::slot_type(
              &TraderSummaryView::refresh, this, boost::placeholders::_1))),
      traderName(trader.traderName()) {}

void ui::TraderSummaryView::refresh(midas::TradeSummary summary) {
  std::scoped_lock summaryLock(summaryMutex);
  currentSummary = summary;
  updatedAt = std::chrono::duration_cast<std::chrono::seconds>(
      std::chrono::utc_clock::now().time_since_epoch());
}

Component ui::TraderSummaryView::paint() {
  auto renderer = Renderer([this] {
    if (!currentSummary.has_value()) {
      auto document = text("No summary received");
      return document;
    }
    std::vector<Element> fields;
    fields.push_back(text("# entry orders: " +
                          std::to_string(currentSummary->numberOfEntryOrders)));
    fields.push_back(
        text("# stop loss triggers: " +
             std::to_string(currentSummary->numberOfStopLossTriggered)));
    fields.push_back(
        text("# profit triggers: " +
             std::to_string(currentSummary->numberOfProfitTakersTriggered)));
    std::stringstream formatter;
    formatter.imbue(std::locale("en_PT"));
    formatter << "success ratio: " << std::setprecision(2)
              << currentSummary->successRatio;
    fields.push_back(text(formatter.str()));
    std::stringstream().swap(formatter);
    formatter << "max down turn: "
              << std::put_money(currentSummary->maxDownTurn);
    fields.push_back(text(formatter.str()));
    std::stringstream().swap(formatter);
    formatter << "max up turn: " << std::put_money(currentSummary->maxUpTurn);
    fields.push_back(text(formatter.str()));
    std::stringstream().swap(formatter);
    formatter << "current balance: "
              << std::put_money(currentSummary->endingBalance);
    std::stringstream().swap(formatter);
    formatter << "has open position? " << std::boolalpha
              << currentSummary->hasOpenPosition;
    fields.push_back(text(formatter.str()));
    auto document = window(text(traderName), vbox(fields));
    return document;
  });
  lastPaintedAt = std::chrono::duration_cast<std::chrono::seconds>(
      std::chrono::utc_clock::now().time_since_epoch());
  return renderer;
}

bool ui::TraderSummaryView::dirty() const {
  if (!lastPaintedAt.has_value() || !updatedAt.has_value()) {
    return true;
  }
  return lastPaintedAt <= updatedAt;
}