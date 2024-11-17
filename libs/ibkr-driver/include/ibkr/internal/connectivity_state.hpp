#pragma once
#include "CommonDefs.h"
#include "EClientSocket.h"
#include "EReader.h"
#include "EReaderOSSignal.h"
#include <atomic>
#include <boost/asio.hpp>
#include <boost/signals2.hpp>
#include <ostream>
namespace ibkr::internal {
/**
 * Before we are ready to send requests to IBKR API
 * We need more than just a socket connection.
 * We must also track data farm connections
 * as well as having a valid next order id.
 * This class encapsulates that logic
 */
class ConnectivityState {
  typedef boost::signals2::signal<void(bool)> connection_signal_t;

public:
  /**
   *  next valid order id that can be used
   */
  std::atomic<OrderId> nextValidId{-1};
  /**
   * Signal / mutex pthreads based
   */
  EReaderOSSignal readerSignal;
  std::unique_ptr<EClientSocket> clientSocket;
  /**
   * Order of appearance is important so that reader is destructed
   * before socket
   */
  std::unique_ptr<EReader> reader;

  ConnectivityState(EWrapper *);
  ~ConnectivityState();

  void connect(const boost::asio::ip::tcp::endpoint &endpoint);
  void disconnect();

  void notifySecDefServerState(bool connected);
  void notifyDataFarmState(bool connected);
  void notifyHistoricalDataFarmState(bool connected);
  void notifyManagedAccountsReceived();

  inline bool receivedFirstValidId() const { return nextValidId.load() != -1; }

  inline bool ready() const {
    return receivedManagedAccounts && securityDefinitionServerOk &&
           receivedFirstValidId() && connectedDataFarmsCount > 0 &&
           connectedHistoricalDataFarmsCount > 0 && clientSocket->isConnected();
  }
  friend std::ostream &operator<<(std::ostream &stream,
                                  const ConnectivityState &state) {
    std::string validIdState =
        state.receivedFirstValidId()
            ? std::to_string(state.nextValidId.load(std::memory_order_relaxed))
            : " no order id received";

    stream << "[ready: " << state.ready() << "]"
           << "[ next valid order id: " << validIdState << "]"
           << "[ count farms: "
           << state.connectedDataFarmsCount.load(std::memory_order::relaxed)
           << " ][ count historical farms: "
           << state.connectedHistoricalDataFarmsCount.load(
                  std::memory_order::relaxed)
           << " ]";
    return stream;
  }
  boost::signals2::connection
  addConnectListener(const connection_signal_t::slot_type &obs) {
    return connectionSignal.connect(obs);
  }

private:
  bool receivedManagedAccounts = false, securityDefinitionServerOk = false;
  std::atomic<int> connectedDataFarmsCount, connectedHistoricalDataFarmsCount;
  connection_signal_t connectionSignal;
};
} // namespace ibkr::internal