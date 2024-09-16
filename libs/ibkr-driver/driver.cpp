#include "ibkr-driver/ibkr.hpp"
#include "ibkr/internal/client.hpp"

ibkr::Driver::Driver(boost::asio::ip::tcp::endpoint endpoint)
    : implementation{std::make_unique<internal::Client>(endpoint)} {}

void ibkr::Driver::addConnectListener(const std::function<void()> &func) {
  implementation->addConnectListener(func);
}

bool ibkr::Driver::processCycle() { return implementation->processCycle(); }

void ibkr::Driver::addSubscription(subscription_ptr_t subscription) {
  implementation->addSubscription(subscription);
}

void ibkr::Driver::connect() { implementation->connect(); }

void ibkr::Driver::disconnect() { implementation->disconnect(); }

bool ibkr::Driver::isConnected() const { return implementation->isConnected(); }

ibkr::Driver::~Driver() = default;