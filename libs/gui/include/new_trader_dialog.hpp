//
// Created by potato on 29-12-2024.
//

#ifndef NEW_TRADER_DIALOG_HPP
#define NEW_TRADER_DIALOG_HPP
#include "broker-interface/instruments.hpp"

#include <QComboBox>
#include <QDialog>
#include <QLineEdit>
#include <trader/trader.hpp>
#undef emit // tbb compatability
namespace gui {
class NewTraderDialog : public QDialog {
  Q_OBJECT
public:
  explicit NewTraderDialog(QWidget *parent = nullptr);

  struct data_t {
    int quantity;
    midas::InstrumentEnum instrument;
    midas::trader::TraderType traderType;
  };
  [[nodiscard]] inline std::optional<data_t> getData() const { return data; }
  void accept() override;
  void reject() override;

private:
  QComboBox *instrumentComboBox, *traderTypeComboBox;
  QLineEdit *entryQuantity;
  std::optional<data_t> data;
};
} // namespace gui
#endif // NEW_TRADER_DIALOG_HPP
