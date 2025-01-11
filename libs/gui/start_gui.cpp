//
// Created by potato on 29-12-2024.
//
#include "gui/gui.hpp"
#include "main_window.hpp"
#include <QApplication>

void gui::start(int argc, char **argv) {
  std::atomic<bool> quitSignal{false};
  QApplication app(argc, argv);
  QLocale::setDefault(QLocale::c());
  std::locale::global(std::locale::classic());
  MainWindow w(&quitSignal);
  w.show();

  app.exec();
}