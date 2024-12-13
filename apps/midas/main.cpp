
#include "backtest/momentum_trader.hpp"
#include "logging/logging.hpp"
#include "midas/version.h"
#include "signal-handling/signal-handler.hpp"
#include "terminal-ui/teriminal.hpp"

#include <boost/program_options.hpp>
#include <iostream>
namespace po = boost::program_options;

static po::options_description appOptionsDesc("Midas AlgoTrading options");

static po::variables_map parseCmdLine(int argc, char *argv[]) {
  appOptionsDesc.add_options()("help,h", "prints this message")(
      "version,v", "prints program version")("simulate,-s", "simulation mode")(
      "live,-l", "live trading mode");
  po::variables_map vm;
  po::store(po::parse_command_line(argc, argv, appOptionsDesc), vm);
  po::notify(vm);
  return vm;
}

int main(int argc, char *argv[]) {
  logging::initialize_logging();
  midas::SignalHandler signalHandler;
  auto logger = logging::create_channel_logger("cmdline");
  INFO_LOG(logger) << "Midas v" << MIDAS_VERSION << " starting";
  auto vm = parseCmdLine(argc, argv);

  if (vm.count("help")) {
    std::cout << appOptionsDesc << '\n';
  } else if (vm.count("version")) {
    std::cout << "Midas version v" << MIDAS_VERSION << '\n';
  } else if (vm.count("simulate")) {
    // simulate
    backtestMomentumTrader();
  } else if (vm.count("live")) {
    ui::startTerminalUI(signalHandler);
  } else {
    std::cerr
        << "This program requires at least one command. Run midas -h for help"
        << std::endl;
  }

  return 0;
}