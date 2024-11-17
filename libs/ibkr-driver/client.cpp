#include "ibkr/internal/client.hpp"
#include <atomic>
#include <mutex>
#include <utility>

ibkr::internal::Client::Client(const boost::asio::ip::tcp::endpoint &endpoint)
    : connectionState(this), endpoint(endpoint),
      logger(logging::create_channel_logger("ibkr-driver")), nextTickerId(1) {}

ibkr::internal::Client::~Client() { disconnect(); }

void ibkr::internal::Client::nextValidId(OrderId order) {
  connectionState.nextValidId.store(order, std::memory_order_relaxed);
}

void ibkr::internal::Client::disconnect() { connectionState.disconnect(); }

void ibkr::internal::Client::connect() { connectionState.connect(endpoint); }

bool ::ibkr::internal::Client::isConnected() const {
  return connectionState.clientSocket->isConnected();
}

bool ibkr::internal::Client::processCycle() {
  // first we handle api messages
  connectionState.readerSignal.waitForSignal();
  connectionState.reader->processMsgs();

  if (!connectionState.ready()) {
    return false;
  }
  // Subscriptions
  processPendingSubscriptions();
  // Order Transmission
  processPendingOrders();
  // commands
  processPendingCommands();
  return false;
}

void ibkr::internal::Client::processPendingCommands() {
  std::list<std::function<void()>> batch;
  {
    std::scoped_lock lock(commandsMutex);
    std::swap(batch, pendingCommands);
  }
  for (auto &command : batch) {
    command();
  }
}
