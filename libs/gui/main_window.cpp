//
// Created by potato on 29-12-2024.
//

#include "main_window.hpp"

#include "backtest_widget.hpp"

#include <QMenuBar>
#include <new_trader_dialog.hpp>
#include <trader/trader.hpp>
#include <trader_widget.hpp>

gui::MainWindow::MainWindow(std::atomic<bool> *quitSignal)
    : quitSignal(quitSignal), tradingContext(std::make_shared<midas::TradingContext>(quitSignal)) {
  setMinimumSize(600, 400);
  traderTabs = new QTabWidget(this);
  setCentralWidget(traderTabs);
  QMenu *menu = menuBar()->addMenu("Traders");
  QAction *newTraderAction = menu->addAction("New Active");
  newTraderAction->setShortcut(QKeySequence::New);
  newTraderAction->setStatusTip("Create a new active trader");
  connect(newTraderAction, SIGNAL(triggered()), this, SLOT(newTrader()));

  QAction *newBacktestAction = menu->addAction("New Backtest");
  newBacktestAction->setStatusTip("Start a new backtest");
  connect(newBacktestAction, SIGNAL(triggered()), this, SLOT(newBacktest()));
}

void gui::MainWindow::newActiveTrader() {
  NewTraderDialog dialog(this);
  dialog.exec();

  auto data = dialog.getData();
  traders.emplace_back(std::make_shared<midas::TraderContext>(
      quitSignal, 100 * 120, tradingContext.get(), data->instrument,
      data->quantity));
  TraderWidget *traderWidget = new TraderWidget(traders.back());
  traderTabs->addTab(traderWidget, traderWidget->getName());
}


void gui::MainWindow::newBacktest() {
  NewTraderDialog dialog(this);
  dialog.exec();

  auto data = dialog.getData();
 BacktestWidget *backtestWidget = new BacktestWidget(tradingContext, data->instrument, data->quantity, traderTabs);
  traderTabs->addTab(backtestWidget, backtestWidget->getName());
}

gui::MainWindow::~MainWindow() { quitSignal->store(true); }