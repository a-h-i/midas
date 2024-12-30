//
// Created by potato on 30-12-2024.
//

#ifndef BACKTEST_WIDGET_HPP
#define BACKTEST_WIDGET_HPP
#include <QLabel>
#include <QWidget>
#undef emit

#include <trader/trader_context.hpp>
namespace gui {
class BacktestWidget: public QWidget {
  Q_OBJECT
public:
  explicit BacktestWidget(const std::shared_ptr<midas::TradingContext> &context,
    midas::InstrumentEnum instrument, int entryQuantity, QWidget *parent = nullptr);
  QString getName() const;
private:
  std::shared_ptr<midas::TradingContext> context;
  midas::InstrumentEnum instrument;
  int entryQuantity;
  std::jthread thread;
  QLabel *content;
};
}

#endif //BACKTEST_WIDGET_HPP
