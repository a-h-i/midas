//
// Created by potato on 29-12-2024.
//

#include "main_window.hpp"
#include <QMenuBar>
#include <new_trader_dialog.hpp>
#include <trader_widget.hpp>
#include <trader/trader.hpp>

gui::MainWindow::MainWindow(std::atomic<bool> *quitSignal)
    : quitSignal(quitSignal), tradingContext(quitSignal) {
  setMinimumSize(600, 400);
  traderTabs = new QTabWidget(this);
  setCentralWidget(traderTabs);
  QMenu *menu = menuBar()->addMenu("Traders");
  QAction *newAction = menu->addAction("New");
  newAction->setShortcut(QKeySequence::New);
  newAction->setStatusTip("Create a new trader");
  connect(newAction, SIGNAL(triggered()), this, SLOT(newTrader()));
}

void gui::MainWindow::newTrader() {
  NewTraderDialog dialog(this);
  dialog.exec();

  auto data = dialog.getData();
  traders.emplace_back(std::make_shared<midas::TraderContext>(
      quitSignal, 100 * 120, &tradingContext, data->instrument,
      data->quantity));
  TraderWidget *traderWidget = new TraderWidget(traders.back());
  traderTabs->addTab(traderWidget, traderWidget->getName());
}
gui::MainWindow::~MainWindow() { quitSignal->store(true); }
