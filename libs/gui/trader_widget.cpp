//
// Created by potato on 30-12-2024.
//
#include "trader_widget.hpp"

#include <QPushButton>
#include <QVBoxLayout>

using namespace gui;

TraderWidget::TraderWidget(const std::shared_ptr<midas::TraderContext> &context)
    : context(context),
      traderSummaryConnection(context->trader->connectSummarySlot(
          midas::trader::Trader::trade_summary_signal_t::slot_type(
              &TraderWidget::refresh, this, boost::placeholders::_1))) {

  QVBoxLayout *layout = new QVBoxLayout(this);
  layout->setSpacing(2);
  summaryLayout = new QVBoxLayout(this);
  entryOrdersTriggered = new QLabel("Entry orders: 0", this);
  stopLossTriggered = new QLabel("Loss triggers: 0", this);
  profitTakersTriggered = new QLabel("Profit triggers: 0", this);
  successRatio = new QLabel("Ratio: 0%", this);
  hasOpenPosition = new QLabel("Open position? false", this);
  isPaused = new QLabel(
      "Paused? " + QString(context->trader->paused() ? "true" : "false"), this);
  summaryLayout->addWidget(entryOrdersTriggered);
  summaryLayout->addWidget(stopLossTriggered);
  summaryLayout->addWidget(profitTakersTriggered);
  summaryLayout->addWidget(successRatio);
  summaryLayout->addWidget(hasOpenPosition);

  layout->addItem(summaryLayout);

  QHBoxLayout *buttonLayout = new QHBoxLayout();

  QPushButton *pauseButton = new QPushButton("Pause");
  buttonLayout->addWidget(pauseButton);
  connect(pauseButton, SIGNAL(clicked()), this, SLOT(slotPause()));

  layout->addLayout(buttonLayout);
  this->setLayout(layout);
}

void TraderWidget::slotPause() {
  context->trader->togglePause();
  isPaused->setText("Paused? " +
                    QString(context->trader->paused() ? "true" : "false"));
}

void TraderWidget::refresh(midas::TradeSummary summary) {
  std::scoped_lock lock(mutex);
  entryOrdersTriggered->setText(
      "Entry Orders: " + QString::number(summary.numberOfEntryOrdersTriggered));
  stopLossTriggered->setText(
      "Stop Loss: " + QString::number(summary.numberOfStopLossTriggered));
  profitTakersTriggered->setText(
      "Profit triggers: " +
      QString::number(summary.numberOfProfitTakersTriggered));
  successRatio->setText(
      "Ratio: " + QString::number(summary.successRatio * 100, 'f', 3) + "%");
  hasOpenPosition->setText("Open Position? " +
                           QString(summary.hasOpenPosition ? "true" : "false"));
  isPaused->setText("Paused? " +
                    QString(context->trader->paused() ? "true" : "false"));
}

QString TraderWidget::getName() const {
  return QString::fromStdString(context->trader->traderName());
}