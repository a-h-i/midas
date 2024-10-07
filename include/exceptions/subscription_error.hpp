#pragma once
#include <stdexcept>

class SubscriptionError : public std::logic_error::logic_error {
    public:
    SubscriptionError(const std::string &message): logic_error(message) {}
};