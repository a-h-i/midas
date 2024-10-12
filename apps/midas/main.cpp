
#include "backtest/backtest.hpp"
#include "broker-interface/broker.hpp"
#include "broker-interface/instruments.hpp"
#include "broker-interface/order.hpp"
#include "data/data_stream.hpp"
#include "ibkr-driver/ibkr.hpp"
#include "logging/logging.hpp"
#include "midas/version.h"
#include "trader/trader.hpp"
#include <atomic>
#include <boost/asio/ip/address.hpp>
#include <boost/program_options.hpp>
#include <condition_variable>
#include <csignal>
#include <fstream>
#include <ios>
#include <iostream>
#include <memory>
#include <mutex>
#include <thread>

using namespace std::chrono_literals;
namespace po = boost::program_options;
namespace backtest = midas::backtest;
using namespace backtest::literals;

static po::options_description appOptionsDesc("Midas AlgoTrading options");

static po::variables_map parseCmdLine(int argc, char *argv[]) {
  appOptionsDesc.add_options()("help,h", "prints this message")(
      "version,v", "prints program version");
  po::variables_map vm;
  po::store(po::parse_command_line(argc, argv, appOptionsDesc), vm);
  po::notify(vm);
  return vm;
}

static void backtestMomentumTrader() {
  auto traderFactory = [](std::shared_ptr<midas::DataStream> streamPtr,
                          std::shared_ptr<midas::OrderManager> orderManager) {
    return midas::trader::momentumExploit(
        streamPtr, orderManager, midas::InstrumentEnum::MicroNasdaqFutures);
  };

  boost::asio::ip::tcp::endpoint ibkrServer(
      boost::asio::ip::make_address("127.0.0.1"), 7496);
  std::unique_ptr<midas::Broker> broker(new ibkr::Driver(ibkrServer));
  broker->connect();
  std::atomic_bool driverWorkerTermination{false};
  std::jthread brokerProcessor([&broker, &driverWorkerTermination] {
    while (!driverWorkerTermination.load()) {
      broker->processCycle();
      std::this_thread::sleep_for(100ms);
    }
  });
  backtest::BacktestResult results =
      backtest::performBacktest(midas::InstrumentEnum::MicroNasdaqFutures,
                                2_months, traderFactory, *broker);
  std::cout << "Trade Summary"
            << "\nnumber  entry orders: " << results.summary.numberOfEntryOrders
            << "\nnumber of stop loss orders triggered "
            << results.summary.numberOfStopLossTriggered
            << "\nnumber of profit takers triggered "
            << results.summary.numberOfProfitTakersTriggered
            << "\nsuccess ratio " << results.summary.successRatio
            << "\nmax down turn " << results.summary.maxDownTurn
            << "\nmax up turn " << results.summary.maxUpTurn
            << "\nending balance " << results.summary.endingBalance;
  std::ofstream sourceCsv("source.csv", std::ios::out),
      downSampledCsv("down_sampled.csv", std::ios::out);
  sourceCsv << results.originalStream;
  downSampledCsv << results.traderDownSampledStream;
  driverWorkerTermination.store(true);
}

int main(int argc, char *argv[]) {
  logging::initialize_logging();
  // sigset_t sigset;
  // sigemptyset(&sigset);
  // Only listen to interrupts and terminations
  // sigaddset(&sigset, SIGINT);
  // sigaddset(&sigset, SIGTERM);
  // pthread_sigmask(SIG_BLOCK, &sigset, nullptr);
  std::atomic<bool> terminationRequested(false);
  std::mutex signalHandlingCvMutex;
  std::condition_variable signalHandlingCv;

  auto logger = logging::create_channel_logger("cmdline");
  INFO_LOG(logger) << "Midas v" << MIDAS_VERSION << " starting";
  auto vm = parseCmdLine(argc, argv);

  // std::jthread signalHandler(
  //     [&sigset, &terminationRequested, &signalHandlingCv, &logger]() {
  //       int signalNumber = 0;
  //       timespec sigTimeout;
  //       sigTimeout.tv_sec = 0;
  //       sigTimeout.tv_nsec = 500000000;
  //       do {
  //         signalNumber = sigtimedwait(&sigset, nullptr, &sigTimeout);
  //       } while (!terminationRequested.load() && signalNumber == -1 &&
  //                errno == EAGAIN);
  //       INFO_LOG(logger) << "Received termination signal";
  //       terminationRequested.store(true);
  //       signalHandlingCv.notify_all();
  //     });

  if (vm.count("help")) {
    std::cout << appOptionsDesc << '\n';
  } else if (vm.count("version")) {
    INFO_LOG(logger) << "Midas version v" << MIDAS_VERSION << '\n';
  } else {
    // simulate
    backtestMomentumTrader();
  }
  terminationRequested.store(true);
  signalHandlingCv.notify_all();
  return 0;
}