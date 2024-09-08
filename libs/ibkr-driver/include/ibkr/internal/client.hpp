#pragma once

#include "EClientSocket.h"
#include "EReader.h"
#include "EReaderOSSignal.h"
#include "EWrapper.h"

#include <boost/asio.hpp>
#include <memory>
#include <string>

namespace ibkr::internal {
class Client : public EWrapper {

public:
  Client(const boost::asio::ip::tcp::endpoint &endpoint);
  virtual ~Client();

  // Event Handling
  virtual void nextValidId(OrderId order);
  virtual void error(int id, int errorCode, const std::string& errorString, const std::string& advancedOrderRejectJson);

private:
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
  boost::asio::ip::tcp::endpoint endpoint;

  OrderId nextValidOrderId;
};
} // namespace ibkr::internal
