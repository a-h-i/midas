#include "signal-handling/signal-handler.hpp"
#include <csignal>
#include <ctime>
#include <mutex>
#include <thread>

using namespace midas;
using namespace std::chrono_literals;
SignalHandler::SignalHandler() {

  // must be done on main thread
  sigemptyset(&sigset);
  // Only listen to interrupts and terminations
  sigaddset(&sigset, SIGINT);
  sigaddset(&sigset, SIGTERM);
  pthread_sigmask(SIG_BLOCK, &sigset, nullptr);
  handlerThreads.push_back(std::jthread([this]() {
    do {
      int signalNumber = 0;
      timespec sigTimeout;
      sigTimeout.tv_sec = 0;
      sigTimeout.tv_nsec = 500000000;
      do {
        signalNumber = sigtimedwait(&sigset, nullptr, &sigTimeout);
      } while (!terminationRequested.load() && signalNumber == -1 &&
               errno == EAGAIN);

      if (signalNumber == SIGINT) {
        sigintReceived.store(true);
      } else if (signalNumber == SIGTERM) {
        sigtermReceived.store(true);
      }
      signalHandlingCv.notify_all();
    } while (!terminationRequested.load());
  }));

  handlerThreads.push_back(std::jthread([this]() {
    do {
      std::unique_lock signalsLock(signalHandlingCvMutex);

      signalHandlingCv.wait_for(signalsLock, 500ms);
      if (sigtermReceived.load()) {
        terminationSignal();
      } else if (sigintReceived.load()) {
        interruptSignal();
      }
    } while (!terminationRequested.load());
  }));
}

SignalHandler::~SignalHandler() {
  terminationRequested.store(true);
  signalHandlingCv.notify_all();
}

boost::signals2::connection
SignalHandler::addInterruptListener(const signal_t::slot_type &handler) {
  return interruptSignal.connect(handler);
}
boost::signals2::connection
SignalHandler::addTerminationListener(const signal_t::slot_type &handler) {
  return terminationSignal.connect(handler);
}
