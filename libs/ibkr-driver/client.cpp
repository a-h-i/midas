#include "ibkr/internal/client.hpp"
#include "Contract.h"
#include "TagValue.h"
#include "exceptions/network_error.hpp"
#include <sstream>

ibkr::internal::Client::Client(const boost::asio::ip::tcp::endpoint &endpoint)
    : connectionState(this), endpoint(endpoint),
      logger(logging::create_channel_logger("ibkr-driver")) {}

ibkr::internal::Client::~Client() { disconnect(); }

void ibkr::internal::Client::nextValidId(OrderId order) {
  connectionState.nextValidId = order;
}

void ibkr::internal::Client::disconnect() { connectionState.disconnect(); }

void ibkr::internal::Client::connect() { connectionState.connect(endpoint); }

bool ibkr::internal::Client::processCycle() {
  connectionState.reader->processMsgs();
  INFO_LOG(logger) << "State : " << connectionState;
  return false;
}

bool ::ibkr::internal::Client::isConnected() const {
  return connectionState.clientSocket->isConnected();
}