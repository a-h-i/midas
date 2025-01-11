//
// Created by ahi on 1/11/25.
//
#include "pnl_widget.hpp"
using namespace gui;

PnlWidget::PnlWidget(const std::shared_ptr<midas::TradingContext> &context,
                     QWidget *parent)
    : QWidget(parent), context(context),
      pnlConnection(context->orderManager->positionTracker.connectRealizedPnl(
          midas::PositionTracker::realized_pnl_signal_t::slot_type(
              &PnlWidget::refresh, this))) {
  setMinimumSize({600, 400});
  pnlLayout = new QVBoxLayout(this);
  setLayout(pnlLayout);

}


void PnlWidget::refresh() {
  std::scoped_lock lock(mutex);
  pnlLayout = new QVBoxLayout(this);
  setLayout(pnlLayout);
  auto pnl = context->orderManager->positionTracker.getPnl();
  for (auto &[instrument, position] : pnl) {
    auto *layout = new QHBoxLayout(this);
    QString name = QString::fromStdString(midas::to_string(instrument));
    QString pnl = QString::number(position);
    layout->addWidget(new QLabel(name));
    layout->addWidget(new QLabel(pnl));
    pnlLayout->addLayout(layout);
  }
}