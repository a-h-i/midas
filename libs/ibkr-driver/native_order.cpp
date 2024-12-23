#include "Decimal.h"
#include "ibkr/internal/build_contracts.hpp"
#include "ibkr/internal/order_wrapper.hpp"
#include "logging/logging.hpp"
#include <algorithm>
#include <boost/unordered/unordered_flat_set.hpp>
#include <iterator>
#include <numeric>
#include <utility>
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

NativeOrder::NativeOrder(Order native, midas::Order &order, std::shared_ptr<logging::thread_safe_logger_t> & logger)
    : logger(logger), events(std::make_unique<NativeOrderSignals>(order)), nativeOrder(std::move(native)),
      instrument(order.instrument),
      ibkrContract(ibkr::internal::build_futures_contract(order.instrument)) {}

bool NativeOrder::addCommissionEntry(const CommissionEntry &entry) {
  std::scoped_lock lock(stateMutex);
  const auto pos = std::ranges::find_if(executions,
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
  std::scoped_lock lock(stateMutex);
  std::erase_if(executions, [this](const ExecutionEntry &execution) {
    const auto baseId = execution.getBaseId();
    // we want to remove if others have same id and this doesn't correct others
    return std::ranges::any_of(
        executions,
                       [&execution, &baseId](const ExecutionEntry &other) {
                         return baseId == other.getBaseId() &&
                                !execution.corrects(other);
                       });
  });
}

void NativeOrder::checkCommissionsComplete() {
  std::scoped_lock lock(stateMutex);
  // we don't really care about commissions right now
  // TODO: actually implement
  std::atomic_thread_fence(std::memory_order_acq_rel);
  state.commissionsCompletelyReceived.store(true);
}



void NativeOrder::addExecutionEntry(const ExecutionEntry &execution) {
  std::scoped_lock lock(stateMutex);

  executions.push_back(execution);
  double totalExecuted = std::accumulate(
      std::begin(executions), std::end(executions), 0.0,
      [](const auto &lhs, const auto &rhs) { return lhs + rhs.quantity; });
  std::atomic_thread_fence(std::memory_order_acq_rel);
  state.commissionsCompletelyReceived.store(false);
  state.executionsCompletelyReceived.store(
      totalExecuted >=
      DecimalFunctions::decimalToDouble(nativeOrder.totalQuantity));
  checkCommissionsComplete();
  processStateChange();
}

void NativeOrder::processStateChange() {
  std::scoped_lock lock(stateMutex);
  if (!inCompletelyFilledState() || executions.empty()) {
    return;
  }
  std::atomic_thread_fence(std::memory_order_acq_rel);
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
  INFO_LOG(*logger) << "Signalling filled event: avgFill:  " << avgFill << " totalCommissions: " << totalCommissions << " quantity: " << filledQuantity;
  events->setFilled(avgFill, totalCommissions, filledQuantity);
}