//
// Created by potato on 29-12-2024.
//
#include "new_trader_dialog.hpp"

#include "broker-interface/instruments.hpp"

#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

gui::NewTraderDialog::NewTraderDialog(QWidget *parent) : QDialog(parent) {
  setWindowTitle("New Trader");
  QVBoxLayout *layout = new QVBoxLayout(this);

  QLabel *traderLabel = new QLabel("Trader type", this);
  layout->addWidget(traderLabel);
  traderTypeComboBox = new QComboBox(this);
  traderTypeComboBox->addItems(
      {QString::fromStdString(
           midas::trader::to_string(midas::trader::TraderType::MomentumTrader)),
       QString::fromStdString(midas::trader::to_string(
           midas::trader::TraderType::MeanReversionTrader)),
       QString::fromStdString(
           midas::trader::to_string(midas::trader::TraderType::MACDTrader))});
  layout->addWidget(traderTypeComboBox);

  QLabel *instrumentLabel = new QLabel("Select instrument", this);
  layout->addWidget(instrumentLabel);
  instrumentComboBox = new QComboBox(this);

  std::string russel("" + midas::InstrumentEnum::MicroRussel);
  std::string spx("" + midas::InstrumentEnum::MicroSPXFutures);
  std::string mnq("" + midas::InstrumentEnum::MicroNasdaqFutures);
  std::string nvda("" + midas::InstrumentEnum::NVDA);
  std::string tsla("" + midas::InstrumentEnum::TSLA);

  instrumentComboBox->addItems(
      {russel.c_str(), spx.c_str(), mnq.c_str(), nvda.c_str(), tsla.c_str()});
  layout->addWidget(instrumentComboBox);

  QLabel *quantityLabel = new QLabel("Entry Quantity", this);
  layout->addWidget(quantityLabel);
  entryQuantity = new QLineEdit(this);
  QIntValidator *validator = new QIntValidator(1, 1000, entryQuantity);
  entryQuantity->setValidator(validator);
  layout->addWidget(entryQuantity);

  QPushButton *okButton = new QPushButton("OK", this);
  QPushButton *cancelButton = new QPushButton("Cancel", this);
  layout->addWidget(okButton);
  layout->addWidget(cancelButton);
  connect(okButton, SIGNAL(clicked()), this, SLOT(accept()));
  connect(cancelButton, SIGNAL(clicked()), this, SLOT(reject()));
}

void gui::NewTraderDialog::accept() {
  data_t data{.quantity = entryQuantity->text().toInt(),
              .instrument = midas::stringToInstrument(
                  instrumentComboBox->currentText().toStdString()),
              .traderType = midas::trader::trader_from_string(
                  traderTypeComboBox->currentText().toStdString())};
  this->data = data;
  QDialog::accept();
}

void gui::NewTraderDialog::reject() {
  this->data.reset();
  QDialog::reject();
}