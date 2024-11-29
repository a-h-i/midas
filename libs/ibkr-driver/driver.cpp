#include "ibkr-driver/ibkr.hpp"
#include "ibkr/internal/client.hpp"
#include "ibkr/internal/historical.hpp"
#include "ibkr/internal/ibkr_order_manager.hpp"
#include <atomic>
#include <boost/signals2/connection.hpp>
#include <memory>

ibkr::Driver::Driver(boost::asio::ip::tcp::endpoint endpoint)
    : implementation{std::make_unique<internal::Client>(endpoint)},
      orderManager(new internal::OrderManager(*this)) {}

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

std::atomic<OrderId> &ibkr::Driver::orderCtr() {
  return implementation->orderCtr();
}

unsigned int ibkr::Driver::estimateHistoricalBarSizeSeconds(
    const midas::HistorySubscriptionStartPoint &duration) const {
  return internal::historicalBarSize(duration).sizeSeconds;
}

std::shared_ptr<midas::OrderManager> ibkr::Driver::getOrderManager() {
  return orderManager;
}