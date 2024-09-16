
#include "data/data_stream.hpp"
#include "exceptions/network_error.hpp"
#include "ibkr-driver/ibkr.hpp"
#include "ibkr-driver/known_symbols.hpp"
#include "ibkr-driver/subscription.hpp"
#include "logging/logging.hpp"
#include "midas/instruments.hpp"
#include "midas/version.h"
#include <boost/log/sources/severity_channel_logger.hpp>
#include <boost/log/trivial.hpp>
#include <boost/program_options.hpp>
#include <condition_variable>
#include <csignal>
#include <iostream>
#include <memory>
#include <mutex>
#include <thread>

using namespace std::chrono_literals;
namespace po = boost::program_options;
static po::options_description appOptionsDesc("Midas AlgoTrading options");

static po::variables_map parseCmdLine(int argc, char *argv[]) {
  appOptionsDesc.add_options()("help,h", "prints this message")(
      "version,v", "prints program version");
  po::variables_map vm;
  po::store(po::parse_command_line(argc, argv, appOptionsDesc), vm);
  po::notify(vm);
  return vm;
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

  std::jthread signalHandler([&sigset, &terminationRequested,
                              &signalHandlingCvMutex, &signalHandlingCv]() {
    int signalNumber = 0;
    sigwait(&sigset, &signalNumber);
    {
      std::scoped_lock lock(signalHandlingCvMutex);
      terminationRequested.store(true);
    }
    signalHandlingCv.notify_all();
  });

  boost::log::sources::severity_channel_logger_mt<logging::SeverityLevel,
                                                  std::string>
      lg(boost::log::keywords::channel = "cmdline",
         boost::log::keywords::severity = logging::SeverityLevel::info);
  LOG(lg, logging::SeverityLevel::info)
      << "Midas v" << MIDAS_VERSION << " starting";
  auto vm = parseCmdLine(argc, argv);
  if (vm.empty() || vm.count("help")) {
    std::cout << appOptionsDesc << '\n';
  }

  if (vm.count("version")) {
    std::cout << "Midas version v" << MIDAS_VERSION << '\n';
  }
  try {
    const auto address = boost::asio::ip::make_address("127.0.0.1");
    const auto endpoint = boost::asio::ip::tcp::endpoint(address, 7496);
    ibkr::Driver driver(endpoint);
    midas::DataStream mnqOneWeekChart(30);
    std::shared_ptr<ibkr::Subscription> historySubscription =
        std::make_shared<ibkr::Subscription>(ibkr::Symbols::MNQ, false);

    driver.addSubscription(std::weak_ptr(historySubscription));
    historySubscription->barListeners.add_listener(
        [&mnqOneWeekChart]([[maybe_unused]] const ibkr::Subscription &sub,
                           midas::Bar bar) { mnqOneWeekChart.addBars(bar); });
    driver.connect();
    std::jthread chartWorker([&mnqOneWeekChart, &terminationRequested, &lg] {
      while (terminationRequested == false) {
        mnqOneWeekChart.waitForData(std::chrono::milliseconds(500));
      }
    });
    std::jthread driverProcessor([&driver, &terminationRequested,
                                  &signalHandlingCv, &signalHandlingCvMutex, &lg]() {
      while (terminationRequested == false) {
        driver.processCycle();
        std::unique_lock lock(signalHandlingCvMutex);
        signalHandlingCv.wait_for(
            lock, std::chrono::milliseconds(500),
            [&terminationRequested] { return terminationRequested.load(); });
      }
    });

  } catch (NetworkError &e) {
    std::cerr << "NETWORK error " << e.what() << std::endl;
  }
  return 0;
}