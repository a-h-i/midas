#include "broker-interface/order.hpp"

midas::OrderDirection midas::operator~(OrderDirection original) {
  if (original == OrderDirection::BUY) {
    return OrderDirection::SELL;
  } else {
    return OrderDirection::BUY;
  }
}