//
// Created by potato on 29-12-2024.
//

#ifndef MAIN_WINDOW_HPP
#define MAIN_WINDOW_HPP


#include <QMainWindow>
#undef emit //tbb compatability
#include <trader/base_trader.hpp>
#include <trader/trader_context.hpp>

namespace gui {
class MainWindow : public QMainWindow {
  Q_OBJECT
public:
  MainWindow(std::atomic<bool> *quitSignal);
  virtual ~MainWindow();

private slots:
  void newActiveTrader();
  void newBacktest();
private:
  QTabWidget* traderTabs;
  std::list<std::shared_ptr<midas::TraderContext>> traders;
  std::atomic<bool> *quitSignal;
  std::shared_ptr<midas::TradingContext> tradingContext;

};
}

#endif //MAIN_WINDOW_HPP
