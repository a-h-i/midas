#include "exceptions/network_error.hpp"
#include "ibkr/internal/client.hpp"
#include <algorithm>

inline void farmConnectionStateHelper(std::atomic<int> &counter, bool state) {
  if (state) {
    counter++;
  } else {
    // CAS
    // We don't want to go to negative count
    auto old = counter.load(std::memory_order::relaxed);
    while (!counter.compare_exchange_weak(old, std::max(old - 1, 0),
                                          std::memory_order::release,
                                          std::memory_order::relaxed))
      ;
  }
}

ibkr::internal::Client::ConnectivityState::ConnectivityState(EWrapper *wrapper)
    : readerSignal(2000),
      clientSocket(new EClientSocket(wrapper, &readerSignal)) {
  clientSocket->setConnectOptions("+PACEAPI");
}

ibkr::internal::Client::ConnectivityState::~ConnectivityState() {
  disconnect();
}

void ibkr::internal::Client::ConnectivityState::connect(
    const boost::asio::ip::tcp::endpoint &endpoint) {
  if (clientSocket->isConnected()) {
    return;
  }
  std::ostringstream host;
  host << endpoint.address();
  // attempt to connect
  auto addressView = std::string(host.view());
  bool connected =
      clientSocket->eConnect(addressView.c_str(), endpoint.port(), 1, false);

  if (!connected) {
    host << " port: " << endpoint.port();
    throw NetworkError("IBKR Driver Can not connect to host at " + host.str());
  }
  // Reader initialized after connection due to api version negotiation
  reader = std::make_unique<EReader>(clientSocket.get(), &readerSignal);
  reader->start();
  connectionSignal(true);
}

void ibkr::internal::Client::ConnectivityState::disconnect() {
  if (clientSocket->isConnected()) {
    clientSocket->eDisconnect();
    connectionSignal(false);
  }
}

void ibkr::internal::Client::ConnectivityState::notifySecDefServerState(
    bool connected) {
  securityDefinitionServerOk = connected;
}

void ibkr::internal::Client::ConnectivityState::notifyDataFarmState(
    bool connected) {
  farmConnectionStateHelper(connectedDataFarmsCount, connected);
}

void ibkr::internal::Client::ConnectivityState::notifyHistoricalDataFarmState(
    bool connected) {
  farmConnectionStateHelper(connectedHistoricalDataFarmsCount, connected);
}

void ibkr::internal::Client::ConnectivityState::
    notifyManagedAccountsReceived() {
  receivedManagedAccounts = true;
}
