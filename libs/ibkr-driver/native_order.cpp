#include "ibkr/internal/build_contracts.hpp"
#include "ibkr/internal/order_wrapper.hpp"

using namespace ibkr::internal;

NativeOrderSignals::~NativeOrderSignals() {
  transmitSignal.disconnect_all_slots();
  cancelSignal.disconnect_all_slots();
}

NativeOrderSignals::NativeOrderSignals(midas::Order &order) {
  transmitSignal.connect([&order]() { order.setTransmitted(); });
  cancelSignal.connect([&order]() { order.setCancelled(); });
}

NativeOrder::NativeOrder(Order native, midas::Order &order)
    : events(new NativeOrderSignals(order)), nativeOrder(native),
      instrument(order.instrument), ibkrContract(ibkr::internal::build_futures_contract(order.instrument)) {}