//
// Created by potato on 29-12-2024.
//
#include "gui/gui.hpp"
#include <QApplication>
#include "main_window.hpp"

void gui::start(int argc, char** argv) {
  std::atomic<bool> quitSignal{false};
  QApplication app(argc, argv);
  MainWindow w(&quitSignal);
  w.show();
  app.exec();
}