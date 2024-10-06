#pragma once
#include "broker-interface/order.hpp"

namespace midas::internal {

class BracketedOrderState {
public:
  BracketedOrder *bracketPtr;
  BracketedOrderState(BracketedOrder *bracketPtr) : bracketPtr(bracketPtr) {}
  virtual ~BracketedOrderState() = default;
  virtual bool isModifiable() const;
  virtual midas::OrderStatusEnum state() const = 0;
  virtual bool canTransmit();
};

class BracketUntransmittedState : public BracketedOrderState {
public:
  BracketUntransmittedState(BracketedOrder *bracketPtr)
      : BracketedOrderState(bracketPtr) {}
  virtual bool isModifiable() const override;
  virtual midas::OrderStatusEnum state() const override;
  virtual bool canTransmit() override;
};

class BracketTransmittedState : public BracketedOrderState {
public:
  BracketTransmittedState(BracketedOrder *bracketPtr)
      : BracketedOrderState(bracketPtr) {}

  virtual midas::OrderStatusEnum state() const override;
};

class BracketHoldingPositionState : public BracketedOrderState {
public:
  BracketHoldingPositionState(BracketedOrder *bracketPtr)
      : BracketedOrderState(bracketPtr) {}
  virtual midas::OrderStatusEnum state() const override;
};

class BracketTerminatedState : public BracketedOrderState {
public:
  BracketTerminatedState(BracketedOrder *bracketPtr)
      : BracketedOrderState(bracketPtr) {}
  virtual midas::OrderStatusEnum state() const override;
};
}; // namespace midas::internal
