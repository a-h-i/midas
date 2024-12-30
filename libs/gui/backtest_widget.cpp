//
// Created by potato on 30-12-2024.
//
#include "backtest_widget.hpp"

#include "backtest/backtest.hpp"

#include <qgridlayout.h>
#include <trader/trader.hpp>

using namespace gui;
using namespace midas::backtest::literals;

BacktestWidget::BacktestWidget(
    const std::shared_ptr<midas::TradingContext> &context,
    midas::InstrumentEnum instrument, int entryQuantity, QWidget *parent)
    : QWidget(parent), context(context), instrument(instrument),
      entryQuantity(entryQuantity) {
  content = new QLabel("Waiting for results", this);
  auto layout = new QGridLayout(this);
  content->setAlignment(Qt::AlignCenter);
  layout->addWidget(content, 0, 0, 1, 1, Qt::AlignCenter);
  setLayout(layout);
  thread = std::jthread([this] {
    auto traderFactory =
        [this](std::shared_ptr<midas::DataStream> streamPtr,
               std::shared_ptr<midas::OrderManager> orderManager) {
          return midas::trader::momentumExploit(
              streamPtr, orderManager, this->instrument, this->entryQuantity);
        };
    midas::backtest::BacktestResult results = midas::backtest::performBacktest(
        this->instrument, 10_days, traderFactory, *(this->context->broker));
    std::stringstream formatted;
    formatted << "Trade Summary for " << this->instrument << "\nnumber entry orders: "
              << results.summary.numberOfEntryOrdersTriggered
              << "\nnumber of stop loss orders triggered "
              << results.summary.numberOfStopLossTriggered
              << "\nnumber of profit takers triggered "
              << results.summary.numberOfProfitTakersTriggered
              << "\nsuccess ratio " << results.summary.successRatio
              << "\nmax down turn " << results.summary.maxDownTurn
              << "\nmax up turn " << results.summary.maxUpTurn
              << "\nending balance " << results.summary.endingBalance;
    this->content->setText(QString::fromStdString(formatted.str()));
  });
}

QString BacktestWidget::getName() const {
  return QString("Backtest: ") +
         QString::fromStdString(midas::to_string(instrument));
}