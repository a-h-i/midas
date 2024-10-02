#pragma once
#include "broker-interface/order.hpp"

namespace midas::internal {

class BracketedOrderState {
public:
  const BracketedOrder *bracketPtr;
  BracketedOrderState(BracketedOrder *bracketPtr) : bracketPtr(bracketPtr) {}
  virtual ~BracketedOrderState() = default;
  virtual bool isModifiable() const = 0;
  virtual midas::OrderStatusEnum state() const = 0;
};

class BracketUntransmittedState : public BracketedOrderState {
public:
  BracketUntransmittedState(BracketedOrder *bracketPtr)
      : BracketedOrderState(bracketPtr) {}
  virtual bool isModifiable() const override { return true; }
  virtual midas::OrderStatusEnum state() const override {
    return OrderStatusEnum::UnTransmitted;
  }
};

class BracketTransmittedState : public BracketedOrderState {
public:
  BracketTransmittedState(BracketedOrder *bracketPtr)
      : BracketedOrderState(bracketPtr) {}
  virtual bool isModifiable() const override {
    // currently we are keeping it simple and deeming entire bracket
    // unmodifiable, once entry position is transmitted
    return false;
  }
  virtual midas::OrderStatusEnum state() const override {
    return OrderStatusEnum::UnTransmitted;
  }
};

class BracketHoldingPositionState : public BracketedOrderState {
public:
  BracketHoldingPositionState(BracketedOrder *bracketPtr)
      : BracketedOrderState(bracketPtr) {}
  virtual bool isModifiable() const override {
    // currently we are keeping it simple and deeming entire bracket
    // unmodifiable, once entry position is transmitted
    return false;
  }
};

class BracketTerminatedState : public BracketedOrderState {
public:
  BracketTerminatedState(BracketedOrder *bracketPtr)
      : BracketedOrderState(bracketPtr) {}
  virtual bool isModifiable() const override {
    // currently we are keeping it simple and deeming entire bracket
    // unmodifiable, once entry position is transmitted
    return false;
  }
};
}; // namespace midas::internal
