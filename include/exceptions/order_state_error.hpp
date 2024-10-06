#pragma once
#include <stdexcept>

/**
 * Order state can not be transitioned to
 */
class OrderStateError : public std::logic_error::logic_error {
public:
  OrderStateError(const std::string &message) : logic_error(message) {}
};