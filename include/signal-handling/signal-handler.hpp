#pragma once

#include <atomic>
#include <bits/types/sigset_t.h>
#include <boost/signals2/connection.hpp>
#include <boost/signals2/signal_type.hpp>
#include <condition_variable>
#include <thread>
namespace midas {

/**
Should only be initialized on main thread.
 */
class SignalHandler {
  typedef boost::signals2::signal<void()> signal_t;
  signal_t terminationSignal, interruptSignal;
  std::atomic<bool> sigtermReceived{false}, sigintReceived{false},
      /** used internally to signal exit for background threads */
      terminationRequested{false};
  std::list<std::jthread> handlerThreads;
  sigset_t sigset;
  std::mutex signalHandlingCvMutex;
  std::condition_variable signalHandlingCv;

public:
  SignalHandler();
  ~SignalHandler();
  boost::signals2::connection
  addInterruptListener(const signal_t::slot_type &handler);
  boost::signals2::connection
  addTerminationListener(const signal_t::slot_type &handler);
};
} // namespace midas