
#include "data/data_stream.hpp"
#include "data/export.hpp"
#include "exceptions/network_error.hpp"
#include "ibkr-driver/ibkr.hpp"
#include "ibkr-driver/known_symbols.hpp"
#include "ibkr-driver/subscription.hpp"
#include "logging/logging.hpp"
#include "midas/instruments.hpp"
#include "midas/version.h"
#include "trader/trader.hpp"
#include <boost/log/sources/severity_channel_logger.hpp>
#include <boost/log/trivial.hpp>
#include <boost/program_options.hpp>
#include <condition_variable>
#include <csignal>
#include <iostream>
#include <memory>
#include <mutex>
#include <ranges>
#include <thread>

using namespace std::chrono_literals;
namespace po = boost::program_options;
static po::options_description appOptionsDesc("Midas AlgoTrading options");

static po::variables_map parseCmdLine(int argc, char *argv[]) {
  appOptionsDesc.add_options()("help,h", "prints this message")(
      "version,v", "prints program version")(
      "simulate", po::value<std::string>(),
      "run simulation on historical file")("historical",
                                           "download historical data");
  po::variables_map vm;
  po::store(po::parse_command_line(argc, argv, appOptionsDesc), vm);
  po::notify(vm);
  return vm;
}

static void downloadHistoricalData(std::mutex &signalHandlingCvMutex,
                                   std::condition_variable &signalHandlingCv,
                                   std::atomic<bool> &terminationRequested) {
  auto logger = logging::create_channel_logger("download-historical-data");
  try {
    const auto address = boost::asio::ip::make_address("127.0.0.1");
    const auto endpoint = boost::asio::ip::tcp::endpoint(address, 7496);
    ibkr::Driver driver(endpoint);
    midas::DataStream mnqOneWeekChart(30);
    std::shared_ptr<ibkr::Subscription> historySubscription =
        std::make_shared<ibkr::Subscription>(ibkr::Symbols::MNQ, false);

    driver.addSubscription(std::weak_ptr(historySubscription));
    mnqOneWeekChart.addReOrderListener([&logger] {
      WARNING_LOG(logger) << "reordered";
    });
    historySubscription->barListeners.add_listener(
        [&mnqOneWeekChart]([[maybe_unused]] const ibkr::Subscription &sub,
                           midas::Bar bar) { mnqOneWeekChart.addBars(bar); });
    historySubscription->endListeners.add_listener(
        [&terminationRequested,
         &signalHandlingCv]([[maybe_unused]] const ibkr::Subscription &sub) {
          terminationRequested.store(true, std::memory_order::release);
          signalHandlingCv.notify_all();
        });
    driver.connect();
    std::jthread chartWorker(
        [&mnqOneWeekChart, &terminationRequested, &signalHandlingCv] {
          while (!terminationRequested.load()) {
            mnqOneWeekChart.waitForData(std::chrono::milliseconds(500ms));
          }
        });
    std::jthread driverProcessor([&driver, &terminationRequested,
                                  &signalHandlingCv, &signalHandlingCvMutex]() {
      while (!terminationRequested.load()) {
        driver.processCycle();
        std::unique_lock lock(signalHandlingCvMutex);
        signalHandlingCv.wait_for(
            lock, std::chrono::milliseconds(500ms),
            [&terminationRequested] { return terminationRequested.load(); });
      }
    });

    while (terminationRequested == false) {
      std::unique_lock lock(signalHandlingCvMutex);
      signalHandlingCv.wait_for(
          lock, std::chrono::milliseconds(500ms),
          [&terminationRequested] { return terminationRequested.load(); });
    }
    chartWorker.join();
    const std::string filename =
        "MNQ_" +
        boost::posix_time::to_iso_extended_string(
            boost::posix_time::second_clock::local_time()) +
        ".csv";
    std::ofstream chartStream(filename, std::ios::out | std::ios::trunc);
    chartStream << mnqOneWeekChart << std::endl;
  } catch (NetworkError &e) {
    ERROR_LOG(logger) << "NETWORK error " << e.what() << std::endl;
  }
}

static void runSimulation([[maybe_unused]] const std::string &filePath) {
  auto logger = logging::create_channel_logger("simulator");
  std::ifstream input(filePath, std::ios::in);
  if (!input) {
    ERROR_LOG(logger) << "failed to open file";
    throw std::runtime_error("Failed to Open file " + filePath);
  }
  midas::DataStream simulatedStream(30); // TODO: Specify or infer candle size
  std::string line;
  std::getline(input, line); // header
  while (std::getline(input, line)) {
    // TODO: Read in order specified in header row
    midas::Bar bar;
    line >> bar;
    simulatedStream.addBars(bar);
    // Here we are simulating
    // So we will handle each one by one
    // invoke trader
  }
}

int main(int argc, char *argv[]) {
  logging::initialize_logging();
  sigset_t sigset;
  sigemptyset(&sigset);
  // Only listen to interrupts and terminations
  sigaddset(&sigset, SIGINT);
  sigaddset(&sigset, SIGTERM);
  pthread_sigmask(SIG_BLOCK, &sigset, nullptr);
  std::atomic<bool> terminationRequested(false);
  std::mutex signalHandlingCvMutex;
  std::condition_variable signalHandlingCv;

  auto logger = logging::create_channel_logger("cmdline");
  INFO_LOG(logger) << "Midas v" << MIDAS_VERSION << " starting";
  auto vm = parseCmdLine(argc, argv);

  std::jthread signalHandler([&sigset, &terminationRequested,
                              &signalHandlingCvMutex, &signalHandlingCv,
                              &logger]() {
    int signalNumber = 0;
    timespec sigTimeout;
    sigTimeout.tv_sec = 0;
    sigTimeout.tv_nsec = 500000000;
    do {
      signalNumber = sigtimedwait(&sigset, nullptr, &sigTimeout);
    } while (!terminationRequested.load() && signalNumber == -1 &&
             errno == EAGAIN);
    terminationRequested.store(true);
    signalHandlingCv.notify_all();
  });

  if (vm.count("help")) {
    std::cout << appOptionsDesc << '\n';
  } else if (vm.count("version")) {
    INFO_LOG(logger) << "Midas version v" << MIDAS_VERSION << '\n';
  } else if (vm.count("historical")) {
    downloadHistoricalData(signalHandlingCvMutex, signalHandlingCv,
                           terminationRequested);
  } else if (vm.count("simulate")) {
    runSimulation(vm["simulate"].as<std::string>());
  } else {
    ERROR_LOG(logger) << "No mode specified, use -h for options";
  }
  terminationRequested.store(true);
  signalHandlingCv.notify_all();
  return 0;
}