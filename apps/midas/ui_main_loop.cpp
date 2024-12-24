#include "broker-interface/instruments.hpp"
#include "broker/trader_context.hpp"
#include "terminal-ui/profit_and_loss_window.hpp"
#include "terminal-ui/teriminal.hpp"
#include "terminal-ui/trader_view.hpp"
#include <boost/signals2/connection.hpp>
#include <ftxui/component/component.hpp>
#include <ftxui/component/component_options.hpp>
#include <ftxui/component/loop.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>
#include <ftxui/screen/screen.hpp>
#include <memory>
using namespace ftxui;
using namespace std::chrono_literals;

void ui::startTerminalUI(midas::SignalHandler &globalSignalHandler) {
  std::atomic<bool> quitLoop{false};
  auto screen = ScreenInteractive::Fullscreen();
  try {
    // sync with signal handler for exit
    boost::signals2::scoped_connection terminationConnection =
        globalSignalHandler.addInterruptListener(
            [&quitLoop] { quitLoop.store(true); });

    // Create broker and driver
    TradingContext tradingContext(&quitLoop);
    auto orderManager = tradingContext.orderManager;
    auto broker = tradingContext.broker;

    TraderContext mnqTraderContext(&quitLoop, 100 * 120, &tradingContext,
                                   midas::InstrumentEnum::MicroNasdaqFutures);
    TraderContext mesTraderContext(&quitLoop, 100 * 120, &tradingContext,
                                   midas::InstrumentEnum::MicroSPXFutures);
    auto &mnqTrader = mnqTraderContext.trader;
    auto &mesTrader = mesTraderContext.trader;
    ;
    ProfitAndLossWindow pnlWindow(*orderManager);
    TraderSummaryView mnqSummaryView(*mnqTrader), mesSummaryView(*mesTrader);

    auto pauseBtn = Renderer([&](bool focused) {
      auto btnClr = mnqTrader->paused() ? Color::DarkRed : Color::DarkGreen;
      auto traderPauseButtonOptions = ButtonOption::Animated(btnClr);
      traderPauseButtonOptions.transform = [](const EntryState &s) {
        auto element = text(s.label) | flex;
        if (s.focused) {
          return element | bold | border;
        } else {
          return element | borderEmpty;
        }
      };
      std::string labelStr = mnqTrader->paused() ? "continue" : "pause";
      auto label = text(labelStr) | bgcolor(btnClr) | center | flex;
      if (focused) {
        label = label | bold | border;
      } else {
        label |= borderEmpty;
      }
      return label;
    });
    int selectedTab = 0;

    auto renderer =
        Renderer([&] {
          std::vector<std::string> tabNames{mnqTrader->traderName(),
                                            mesTrader->traderName()};
          auto tabToggle = Toggle(&tabNames, &selectedTab);
          auto traderSummaries = Container::Tab(
              {
                  mnqSummaryView.renderer(),
                  mesSummaryView.renderer(),
              },
              &selectedTab);
          auto traderContainer = Container::Vertical({traderSummaries});
          auto mainView = Container::Vertical({
              traderContainer | flex,
              pauseBtn,

          });
          auto traderWindow = Renderer([&mnqTrader, &traderContainer]() {
            return window(text(mnqTrader->traderName()),
                          traderContainer->Render());
          });
          auto mainWindow = Container::Horizontal(
              {pnlWindow.renderer(), traderWindow | flex});

          return window(text("Midas Algo Trader") | center,
                        mainWindow->Render());
        }) |
        CatchEvent([&mnqTrader, &mesTrader, &selectedTab](Event event) {
          if (event == Event::Character('\n')) {
            mnqTrader->togglePause();
            mesTrader->togglePause();
            return true;
          } else if (event == Event::Character('\t')) {
            selectedTab = (selectedTab + 1) % 2;
            return true;
          }else {
            return false;
          }
        });
    Loop loop(&screen, renderer);
    screen.TrackMouse(true);
    while (!quitLoop.load()) {
      loop.RunOnceBlocking();
    }
  } catch (...) {
    quitLoop.store(true);
    screen.Exit();
  }
}
