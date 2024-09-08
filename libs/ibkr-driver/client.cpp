#include "ibkr/internal/client.hpp"
#include "exceptions/network_error.hpp"

#include <sstream>

ibkr::internal::Client::Client(const boost::asio::ip::tcp::endpoint &endpoint)
    : readerSignal(2000), clientSocket(new EClientSocket(this, &readerSignal)),
      endpoint(endpoint) {

  std::ostringstream host;
  host << endpoint.address();
  // attempt to connect
  bool connected =
      clientSocket->eConnect(host.view().data(), endpoint.port(), 0);

  if (!connected) {
    host << "port: " << endpoint.port();
    throw NetworkError("IBKR Driver Can not connect to host at " + host.str());
  }
}

ibkr::internal::Client::~Client() {
  if (clientSocket->isConnected()) {
    clientSocket->eDisconnect();
  }
}