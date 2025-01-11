//
// Created by ahi on 1/11/25.
//

#ifndef PNL_WIDGET_HPP
#define PNL_WIDGET_HPP

#include <QLabel>
#include <QWidget>
#include <QWindow>
#include <QBoxLayout>
#undef emit
#include "trader/trader_context.hpp"


namespace gui {
class PnlWidget : public QWidget {
  Q_OBJECT
  public:
    explicit PnlWidget(const std::shared_ptr<midas::TradingContext> &context, QWidget *parent);

  private:
    std::shared_ptr<midas::TradingContext> context;
    QVBoxLayout *pnlLayout;
    std::recursive_mutex mutex;
    boost::signals2::scoped_connection pnlConnection;
    void refresh();
};
}
#endif //PNL_WIDGET_HPP
