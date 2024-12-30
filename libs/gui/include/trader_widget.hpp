//
// Created by potato on 30-12-2024.
//

#ifndef TRADER_WIDGET_HPP
#define TRADER_WIDGET_HPP
#include "trader/trader_context.hpp"

#include <QLabel>
#include <QWidget>

#include <QVBoxLayout>
#undef emit

namespace gui {
class TraderWidget : public QWidget {
  Q_OBJECT
public:
  explicit TraderWidget(const std::shared_ptr<midas::TraderContext> &context);
  QString getName() const;

private:
  std::shared_ptr<midas::TraderContext> context;
  boost::signals2::scoped_connection traderSummaryConnection;
  QVBoxLayout *summaryLayout;
  std::recursive_mutex mutex;

private slots:
  void slotPause();

private:
  void refresh(midas::TradeSummary summary);
  QLabel *entryOrdersTriggered, *stopLossTriggered, *profitTakersTriggered,
      *successRatio, *hasOpenPosition, *isPaused;
};
} // namespace gui

#endif // TRADER_WIDGET_HPP
