#pragma once
#include <stdexcept>

class NetworkError : public std::runtime_error::runtime_error {
    public:
    NetworkError(const std::string &message): runtime_error(message) {}
};