#include "broker-interface/broker.hpp"
#include "broker-interface/instruments.hpp"
#include "broker-interface/order.hpp"
#include "broker-interface/subscription.hpp"
#include "broker/broker_factory.hpp"
#include "data/data_stream.hpp"
#include "terminal-ui/profit_and_loss_window.hpp"
#include "terminal-ui/teriminal.hpp"
#include "terminal-ui/trader_view.hpp"
#include "trader/trader.hpp"
#include <boost/signals2/connection.hpp>
#include <ftxui/component/component.hpp>
#include <ftxui/component/component_options.hpp>
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
  std::jthread brokerProcessor([&broker, &quitLoop] {
    while (!quitLoop.load()) {
      broker->processCycle();
    }
  });
  std::shared_ptr<midas::OrderManager> orderManager = broker->getOrderManager();

  std::shared_ptr<midas::DataStream> streamPtr =
      std::make_shared<midas::DataStream>(120);
  auto instrument = midas::InstrumentEnum::MicroNasdaqFutures;
  auto trader =
      midas::trader::momentumExploit(streamPtr, orderManager, instrument);
  std::shared_ptr<midas::Subscription> dataSubscription =
      std::make_shared<midas::Subscription>(instrument, false);
  boost::signals2::scoped_connection barsConnection =
      dataSubscription->barSignal.connect(
          [&streamPtr]([[maybe_unused]] const midas::Subscription &sub,
                       midas::Bar bar) { streamPtr->addBars(bar); });
  boost::signals2::scoped_connection terminationConnection =
      globalSignalHandler.addInterruptListener(
          [&quitLoop] { quitLoop.store(true); });

  broker->addSubscription(dataSubscription);
  auto screen = ScreenInteractive::TerminalOutput();

  ProfitAndLossWindow pnlWindow(*orderManager);
  TraderSummaryView traderSummary(*trader);

  auto renderer = Renderer([&] {
    auto btnClr = trader->paused() ? Color::DarkRed : Color::DarkGreen;
    auto traderPauseButtonOptions = ButtonOption::Animated(btnClr);
    auto traderPauseButton = Button(
        trader->paused() ? "continue" : "pause",
        [&trader] { trader->togglePause(); }, traderPauseButtonOptions);
    auto traderContainer = Container::Vertical({
        traderSummary.render(),
        Renderer([]() { return separator(); }),
        traderPauseButton,

    });
    auto traderWindow =
        window(text(trader->traderName()), traderContainer->Render());
    auto document = pnlWindow.render();
    return window(text("Midas Algo Trader") | center,
                  hbox({
                      pnlWindow.render()->Render(),
                      traderWindow,
                  }));
  });
  Loop loop(&screen, renderer);
  while (!quitLoop.load()) {
    loop.RunOnce();
    std::this_thread::sleep_for(10ms);
  }
}
