#include "terminal-ui/trader_view.hpp"
#include "trader/trader.hpp"
#include <functional>

using midas::trader::Trader;

ui::TraderView::TraderView(Trader &trader) {
  traderSignalConnection = trader.connectSlot(
      std::bind(&TraderView::refresh, this, std::placeholders::_1));
}

ui::TraderView::~TraderView() { traderSignalConnection.disconnect(); }

void ui::TraderView::refresh([[maybe_unused]] midas::TradeSummary summary) {}