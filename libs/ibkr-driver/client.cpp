#include "ibkr/internal/client.hpp"
#include "Contract.h"
#include "TagValue.h"
#include "exceptions/network_error.hpp"
#include <sstream>

ibkr::internal::Client::Client(const boost::asio::ip::tcp::endpoint &endpoint)
    : readerSignal(2000), clientSocket(new EClientSocket(this, &readerSignal)),
      endpoint(endpoint),
      logger(logging::create_channel_logger("ibkr-driver")) {
  clientSocket->setConnectOptions("+PACEAPI");
}

ibkr::internal::Client::~Client() { disconnect(); }

void ibkr::internal::Client::nextValidId(OrderId order) {
  nextValidOrderId = order;
  DEBUG_LOG(logger) << " RECEIVED next valid id " << order;
}

void ::ibkr::internal::Client::disconnect() {
  if (clientSocket->isConnected()) {
    clientSocket->eDisconnect();
  }
}

void ibkr::internal::Client::connect() {
  if (clientSocket->isConnected()) {
    return;
  }
  std::ostringstream host;
  host << endpoint.address();
  // attempt to connect
  bool connected =
      clientSocket->eConnect(host.view().data(), endpoint.port(), 1, false);

  if (!connected) {
    host << "port: " << endpoint.port();
    throw NetworkError("IBKR Driver Can not connect to host at " + host.str());
  }
  // Reader initialized after connection due to api version negotiation
  reader = std::make_unique<EReader>(clientSocket.get(), &readerSignal);
  reader->start();
}

bool ibkr::internal::Client::process_cycle() {
  reader->processMsgs();
  return false;
}

bool ::ibkr::internal::Client::is_connected() const {
  return clientSocket->isConnected();
}