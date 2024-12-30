//
// Created by potato on 29-12-2024.
//
#include "gui/gui.hpp"
#include "logging/logging.hpp"

int main(int argc, char **argv) {
  logging::initialize_logging();
  gui::start(argc, argv);
}
