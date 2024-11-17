#pragma once
#include "Contract.h"
#include "Order.h"
#include "broker-interface/instruments.hpp"
#include "broker-interface/order.hpp"
#include <boost/signals2.hpp>
#include <boost/signals2/variadic_signal.hpp>
#include <memory>
namespace ibkr::internal {

class NativeOrderSignals {
  typedef boost::signals2::signal<void()> transmit_signal_t;
  typedef boost::signals2::signal<void()> cancel_signal_t;
  transmit_signal_t transmitSignal;
  cancel_signal_t cancelSignal;

public:
  NativeOrderSignals(midas::Order &order);
  ~NativeOrderSignals();
};

class NativeOrder {
  std::unique_ptr<NativeOrderSignals> events;

public:
  NativeOrder(Order native, midas::Order &order);
  Order nativeOrder;
  midas::InstrumentEnum instrument;
  Contract ibkrContract;
};
} // namespace ibkr::internal