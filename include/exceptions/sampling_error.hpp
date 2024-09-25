#pragma once
#include <stdexcept>

class SamplingError : public std::logic_error::logic_error {
    public:
    SamplingError(const std::string &message): logic_error(message) {}
};