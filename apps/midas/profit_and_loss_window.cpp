#include "terminal-ui/profit_and_loss_window.hpp"
#include "broker-interface/order.hpp"
#include <ftxui/component/component.hpp>
#include <ftxui/dom/elements.hpp>
#include <iomanip>
#include <locale>
using namespace ui;
using namespace ftxui;
ProfitAndLossWindow::ProfitAndLossWindow(midas::OrderManager &manager) {

  slotConn = manager.addPnLListener(
      [this](double pnlNew) { realizedPnL.store(pnlNew); });
}

Component ProfitAndLossWindow::render() const {

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
  return renderer;
}