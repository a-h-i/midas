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
  // sync with signal handler for exit
  boost::signals2::scoped_connection terminationConnection =
      globalSignalHandler.addInterruptListener(
          [&quitLoop] { quitLoop.store(true); });

  // Create broker and driver
  std::shared_ptr<midas::Broker> broker = createIBKRBroker();
  broker->connect();
  std::jthread brokerProcessor([&broker, &quitLoop] {
    while (!quitLoop.load()) {
      broker->processCycle();
    }
  });
  std::shared_ptr<midas::OrderManager> orderManager = broker->getOrderManager();

  // Subscribe to data stream
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

  broker->addSubscription(dataSubscription);

  std::jthread streamProcessingThread([&trader, &streamPtr, &quitLoop] {
    while (!quitLoop.load()) {
      streamPtr->waitForData(100ms);
      trader->decide();
    }
  });

  // Create UI
  auto screen = ScreenInteractive::Fullscreen();

  ProfitAndLossWindow pnlWindow(*orderManager);
  TraderSummaryView traderSummary(*trader);

  auto pauseBtn = Renderer([&](bool focused) {
    auto btnClr = trader->paused() ? Color::DarkRed : Color::DarkGreen;
    auto traderPauseButtonOptions = ButtonOption::Animated(btnClr);
    traderPauseButtonOptions.transform = [](const EntryState &s) {
      auto element = text(s.label) | flex;
      if (s.focused) {
        return element | bold | border;
      } else {
        return element | borderEmpty;
      }
    };
    std::string labelStr = trader->paused() ? "continue" : "pause";
    auto label = text(labelStr) | bgcolor(btnClr) | center | flex;
    if (focused) {
      label = label | bold | border;
    } else {
      label |= borderEmpty;
    }
    return label;
  });

  auto renderer =
      Renderer([&] {
        auto traderContainer = Container::Vertical({
            traderSummary.renderer() | flex,
            pauseBtn,

        });
        auto traderWindow = Renderer([&trader, &traderContainer]() {
          return window(text(trader->traderName()), traderContainer->Render());
        });
        auto mainWindow =
            Container::Horizontal({pnlWindow.renderer(), traderWindow | flex});

        return window(text("Midas Algo Trader") | center, mainWindow->Render());
      }) |
      CatchEvent([&trader](Event event) {
        if (event == Event::Character('\n')) {
          trader->togglePause();
          return true;
        } else {
          return false;
        }
      });
  Loop loop(&screen, renderer);
  screen.TrackMouse(true);
  while (!quitLoop.load()) {
    loop.RunOnceBlocking();
  }
}
