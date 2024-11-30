#include "broker-interface/broker.hpp"
#include "broker-interface/order.hpp"
#include "broker/broker_factory.hpp"
#include "terminal-ui/profit_and_loss_window.hpp"
#include "terminal-ui/teriminal.hpp"
#include <boost/signals2/connection.hpp>
#include <ftxui/component/component.hpp>
#include <ftxui/component/loop.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>
#include <ftxui/screen/screen.hpp>
#include <memory>
#include <thread>
using namespace ftxui;
using namespace std::chrono_literals;
void ui::startTerminalUI(midas::SignalHandler &globalSignalHandler) {
  std::atomic<bool> quitLoop{false};

  std::shared_ptr<midas::Broker> broker = createIBKRBroker();
  std::shared_ptr<midas::OrderManager> orderManager = broker->getOrderManager();
  ProfitAndLossWindow pnlWindow(*orderManager);
  boost::signals2::scoped_connection terminationConnection =
      globalSignalHandler.addInterruptListener(
          [&quitLoop] { quitLoop.store(true); });

  auto screen = ScreenInteractive::TerminalOutput();

  auto renderer = Renderer([&] {
    auto document = pnlWindow.render();
    return window(text("Midas Algo Trader") | center, hbox(document));
  });
  Loop loop(&screen, renderer);
  while (!quitLoop.load()) {
    loop.RunOnce();
    std::this_thread::sleep_for(10ms);
  }
}
