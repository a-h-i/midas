#pragma once
#include "CommissionReport.h"
#include "Contract.h"
#include "Execution.h"
#include "Order.h"
#include "broker-interface/instruments.hpp"
#include "broker-interface/order.hpp"
#include <boost/date_time/posix_time/ptime.hpp>
#include <boost/signals2.hpp>
#include <boost/signals2/variadic_signal.hpp>
#include <list>
#include <memory>
typedef Execution native_execution_t;
namespace ibkr::internal {

struct CommissionEntry {
  const std::string executionId, currency;
  const double commission, realizedPNL;

  CommissionEntry(const CommissionReport &);
};

/*
 * Orders have more than one execution
 * as they are not filled in an all or nothing manner.
 * Executions can also have corrections.
 */
struct ExecutionEntry {
  const std::string id, exchange;
  const boost::posix_time::ptime serverExecutionTime;
  const midas::OrderDirection direction;
  const double quantity, totalPrice, cumulativeQuantity, averagePrice;
  std::list<CommissionEntry> commissions;

  ExecutionEntry(const native_execution_t &);
  bool corrects(const ExecutionEntry &other) const;
  std::string getBaseId() const;
};

class NativeOrderSignals {
  typedef boost::signals2::signal<void()> transmit_signal_t;
  typedef boost::signals2::signal<void()> cancel_signal_t;
  typedef boost::signals2::signal<void(
      double fillPrice, double totalCommissions, double filledQuantity)>
      fill_signal_t;
  transmit_signal_t transmitSignal;
  cancel_signal_t cancelSignal;
  fill_signal_t fillSignal;

public:
  inline void setTransmitted() { transmitSignal(); }
  inline void setCancelled() { cancelSignal(); }
  inline void setFilled(double price, double commissions, double quantity) {
    fillSignal(price, commissions, quantity);
  }

  NativeOrderSignals(midas::Order &order);
  ~NativeOrderSignals();
};

class NativeOrder {
  std::unique_ptr<NativeOrderSignals> events;
  std::list<ExecutionEntry> executions;

public:
  NativeOrder(Order native, midas::Order &order);
  Order nativeOrder;
  midas::InstrumentEnum instrument;
  Contract ibkrContract;
  /**
  Adds a commission entry if it belongs to the order.
  @returns true if entry belongs to order and was handled.
   */
  bool addCommissionEntry(const CommissionEntry &);
  void cleanCorrectedExecutions();
  inline void setTransmitted() { events->setTransmitted(); }
  inline void setCancelled() { events->setCancelled(); }
  void setCompletelyFilled();
  inline void addExecutionEntry(const ExecutionEntry &execution) {
    executions.push_back(execution);
  }
};
} // namespace ibkr::internal