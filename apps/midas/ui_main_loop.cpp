#include "broker-interface/instruments.hpp"
#include "terminal-ui/profit_and_loss_window.hpp"
#include "terminal-ui/teriminal.hpp"
#include "terminal-ui/trader_view.hpp"
#include "trader/trader_context.hpp"

#include <boost/signals2/connection.hpp>
#include <ftxui/component/component.hpp>
#include <ftxui/component/component_options.hpp>
#include <ftxui/component/loop.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>
#include <memory>
using namespace ftxui;
using namespace std::chrono_literals;

static void createTraders(std::list<midas::TraderContext> &traders,
                          midas::TradingContext *ctx, std::atomic<bool> *stop) {
  auto instruments = {midas::InstrumentEnum::MicroNasdaqFutures};
  for (auto instrument : instruments) {
    traders.emplace_back(stop, 100 * 120, ctx, instrument,
                         midas::getDefaultEntryQuantity(instrument));
  }
}

void ui::startTerminalUI(midas::SignalHandler &globalSignalHandler) {
  std::atomic<bool> quitLoop{false};
  auto screen = ScreenInteractive::Fullscreen();
  try {
    // sync with signal handler for exit
    boost::signals2::scoped_connection terminationConnection =
        globalSignalHandler.addInterruptListener(
            [&quitLoop] { quitLoop.store(true); });

    // Create broker and driver
    midas::TradingContext tradingContext(&quitLoop);
    auto orderManager = tradingContext.orderManager;
    auto broker = tradingContext.broker;
    std::list<midas::TraderContext> traders;
    createTraders(traders, &tradingContext, &quitLoop);
    ;
    ProfitAndLossWindow pnlWindow(*orderManager);
    std::list<TraderSummaryView> summaryViews;
    for (auto &traderContext : traders) {
      summaryViews.emplace_back(*traderContext.trader);
    }
    bool isPaused = false;
    auto pauseBtn = Renderer([&](bool focused) {
      auto btnClr = isPaused ? Color::DarkRed : Color::DarkGreen;
      auto traderPauseButtonOptions = ButtonOption::Animated(btnClr);
      traderPauseButtonOptions.transform = [](const EntryState &s) {
        auto element = text(s.label) | flex;
        if (s.focused) {
          return element | bold | border;
        } else {
          return element | borderEmpty;
        }
      };
      std::string labelStr = isPaused ? "continue" : "pause";
      auto label = text(labelStr) | bgcolor(btnClr) | center | flex;
      if (focused) {
        label = label | bold | border;
      } else {
        label |= borderEmpty;
      }
      return label;
    });
    int selectedTab = 0;
    std::vector<std::string> tabNames;
    for (auto &trader : traders) {
      tabNames.push_back(trader.trader->traderName());
    }
    auto renderer =
        Renderer([&] {
          auto tabToggle = Toggle(&tabNames, &selectedTab);
          std::vector<Component> summaryRenderers;
          for (auto &view : summaryViews) {
            summaryRenderers.push_back(view.renderer());
          }
          auto traderSummaries = Container::Tab(summaryRenderers, &selectedTab);
          auto traderContainer = Container::Vertical({traderSummaries});
          auto mainView = Container::Vertical({
              traderContainer | flex,
              pauseBtn | flex,

          });
          auto traderWindow = Renderer([&traderContainer]() {
            return window(text("Active Traders"), traderContainer->Render());
          });
          auto mainWindow = Container::Horizontal(
              {pnlWindow.renderer(), traderWindow | flex});

          return window(text("Midas Algo Trader") | center,
                        mainWindow->Render());
        }) |
        CatchEvent([&isPaused, &traders, &selectedTab, &tabNames](Event event) {
          if (event == Event::Character('\n')) {
            isPaused = !isPaused;
            for (auto &traderContext : traders) {
              traderContext.trader->togglePause();
            }
            return true;
          } else if (event == Event::Character('\t')) {
            selectedTab = (selectedTab + 1) % tabNames.size();
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
  } catch (...) {
    quitLoop.store(true);
    screen.Exit();
  }
}
