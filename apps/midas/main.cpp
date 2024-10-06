
#include "logging/logging.hpp"
#include "midas/version.h"

#include <boost/program_options.hpp>
#include <condition_variable>
#include <csignal>
#include <iostream>
#include <mutex>
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
                              &signalHandlingCv,
                              &logger]() {
    int signalNumber = 0;
    timespec sigTimeout;
    sigTimeout.tv_sec = 0;
    sigTimeout.tv_nsec = 500000000;
    do {
      signalNumber = sigtimedwait(&sigset, nullptr, &sigTimeout);
    } while (!terminationRequested.load() && signalNumber == -1 &&
             errno == EAGAIN);
    INFO_LOG(logger) << "Received termination signal";
    terminationRequested.store(true);
    signalHandlingCv.notify_all();
    
  });

  if (vm.count("help")) {
    std::cout << appOptionsDesc << '\n';
  } else if (vm.count("version")) {
    INFO_LOG(logger) << "Midas version v" << MIDAS_VERSION << '\n';
  } else if (vm.count("historical")) {
    // 
  } else if (vm.count("simulate")) {
   // simulate
  } else {
    ERROR_LOG(logger) << "No mode specified, use -h for options";
  }
  terminationRequested.store(true);
  signalHandlingCv.notify_all();
  return 0;
}