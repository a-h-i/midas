#include "terminal-ui/profit_and_loss_window.hpp"
#include "broker-interface/order.hpp"
#include <ftxui/component/component.hpp>
#include <ftxui/dom/elements.hpp>
#include <iomanip>
#include <locale>
using namespace ui;
using namespace ftxui;
ProfitAndLossWindow::ProfitAndLossWindow(midas::OrderManager &manager) {

  slotConn = manager.addPnLListener([this](double pnlNew) {
    realizedPnL.store(pnlNew);
    updatedAt = std::chrono::duration_cast<std::chrono::seconds>(
        std::chrono::utc_clock::now().time_since_epoch());
  });
}

Component ProfitAndLossWindow::paint() {

  auto renderer = Renderer([this] {
    auto realizedPnlLabel = text("Realized: ");
    std::stringstream formatter;
    formatter.imbue(std::locale("en_PT"));
    formatter << std::put_money(realizedPnL.load());
    auto realizedPnLValue = text(formatter.str());
    if (realizedPnL.load() < 0) {
      realizedPnLValue |= color(Color::Red);
    } else if (realizedPnL.load() == 0) {
      realizedPnLValue |= color(Color::White);
    } else {
      realizedPnLValue |= color(Color::DarkGreen);
    }
    auto container = vbox({realizedPnlLabel, realizedPnLValue});
    auto document = window(text("PnL") | center, container);
    return document;
  });
  lastPaintedAt = std::chrono::duration_cast<std::chrono::seconds>(
      std::chrono::utc_clock::now().time_since_epoch());
  return renderer;
}

bool ui::ProfitAndLossWindow::dirty() const {
  if (!lastPaintedAt.has_value() || !updatedAt.has_value()) {
    return true;
  }
  return lastPaintedAt <= updatedAt;
}
