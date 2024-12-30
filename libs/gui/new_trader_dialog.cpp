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
  layout->addWidget(okButton);
  connect(okButton, SIGNAL(clicked()), this, SLOT(accept()));
}

void gui::NewTraderDialog::accept() {
  data_t data{.quantity = entryQuantity->text().toInt(),
              .instrument = midas::stringToInstrument(
                  instrumentComboBox->currentText().toStdString())};
  this->data = data;
  QDialog::accept();
}