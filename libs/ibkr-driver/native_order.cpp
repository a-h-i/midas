#include "Decimal.h"
#include "ibkr/internal/build_contracts.hpp"
#include "ibkr/internal/order_wrapper.hpp"
#include <algorithm>
#include <boost/unordered/unordered_flat_set.hpp>
#include <iterator>
#include <numeric>
#include <vector>

using namespace ibkr::internal;

NativeOrderSignals::~NativeOrderSignals() {
  transmitSignal.disconnect_all_slots();
  cancelSignal.disconnect_all_slots();
  fillSignal.disconnect_all_slots();
}

NativeOrderSignals::NativeOrderSignals(midas::Order &order) {
  transmitSignal.connect([&order]() { order.setTransmitted(); });
  cancelSignal.connect([&order]() { order.setCancelled(); });
  fillSignal.connect(
      [&order](double price, double commission, double quantity) {
        order.setFilled(price, commission, quantity);
      });
}

NativeOrder::NativeOrder(Order native, midas::Order &order)
    : events(new NativeOrderSignals(order)), nativeOrder(native),
      instrument(order.instrument),
      ibkrContract(ibkr::internal::build_futures_contract(order.instrument)) {}

bool NativeOrder::addCommissionEntry(const CommissionEntry &entry) {
  auto pos = std::find_if(
      std::begin(executions), std::end(executions),
      [&entry](const auto &exec) { return exec.id == entry.executionId; });
  if (pos == std::end(executions)) {
    return false;
  }
  pos->commissions.push_back(entry);

  checkCommissionsComplete();
  processStateChange();
  return true;
}

void NativeOrder::cleanCorrectedExecutions() {

  std::erase_if(executions, [this](const ExecutionEntry &execution) {
    const auto baseId = execution.getBaseId();
    // we want to remove if others have same id and this doesn't correct others
    return std::any_of(std::begin(executions), std::end(executions),
                       [&execution, &baseId](const ExecutionEntry &other) {
                         return baseId == other.getBaseId() &&
                                !execution.corrects(other);
                       });
  });
}

void NativeOrder::checkCommissionsComplete() {
  // we don't really care about comissions right now
  // TODO: actually implement
  state.commissionsCompletelyReceived.store(true);
}

void NativeOrder::setCompletelyFilled() {
  state.fillEventReceived.store(true);
  processStateChange();
}

void NativeOrder::addExecutionEntry(const ExecutionEntry &execution) {
  state.commissionsCompletelyReceived.store(false);
  executions.push_back(execution);
  auto totalExecuted = std::accumulate(
      std::begin(executions), std::end(executions), 0.0,
      [](const auto &lhs, const auto &rhs) { return lhs + rhs.quantity; });
  state.executionsCompletelyReceived.store(
      totalExecuted >=
      DecimalFunctions::decimalToDouble(nativeOrder.totalQuantity));
  checkCommissionsComplete();
}

void NativeOrder::processStateChange() {

  if (!inCompletelyFilledState()) {
    return;
  }

  double avgFill = std::accumulate(
      std::begin(executions), std::end(executions), 0.0,
      [](const auto &lhs, const auto &rhs) { return lhs + rhs.averagePrice; });
  double totalCommissions = 0;
  double filledQuantity = std::accumulate(
      std::begin(executions), std::end(executions), 0.0,
      [](const auto &lhs, const auto &rhs) { return lhs + rhs.quantity; });

  for (const ExecutionEntry &execution : executions) {

    totalCommissions += std::accumulate(
        std::begin(execution.commissions), std::end(execution.commissions), 0.0,
        [](const auto &first, const auto &second) {
          return first + second.commission;
        });
  }
  avgFill = avgFill / executions.size();
  events->setFilled(avgFill, totalCommissions, filledQuantity);
}