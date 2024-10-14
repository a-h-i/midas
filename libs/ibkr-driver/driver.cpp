#include "ibkr-driver/ibkr.hpp"
#include "ibkr/internal/client.hpp"
#include <boost/signals2/connection.hpp>

ibkr::Driver::Driver(boost::asio::ip::tcp::endpoint endpoint)
    : implementation{std::make_unique<internal::Client>(endpoint)} {}

boost::signals2::connection
ibkr::Driver::addConnectListener(const std::function<void(bool)> &func) {
  return implementation->addConnectListener(func);
}

bool ibkr::Driver::processCycle() { return implementation->processCycle(); }

void ibkr::Driver::addSubscription(
    std::weak_ptr<midas::Subscription> subscription) {
  implementation->addSubscription(subscription);
}

void ibkr::Driver::connect() { implementation->connect(); }

void ibkr::Driver::disconnect() { implementation->disconnect(); }

bool ibkr::Driver::isConnected() const { return implementation->isConnected(); }

ibkr::Driver::~Driver() = default;