#pragma once
#include <stdexcept>

/**
 * Order configuration is malformed or illogical
 */
class OrderParameterError : public std::logic_error::logic_error {
public:
  OrderParameterError(const std::string &message) : logic_error(message) {}
};