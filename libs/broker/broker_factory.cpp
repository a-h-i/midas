

#include "broker-interface/broker.hpp"
#include <ibkr-driver/ibkr.hpp>
#include <memory>


std::shared_ptr<midas::Broker> midas::createIBKRBroker() {
  boost::asio::ip::tcp::endpoint ibkrServer(
      boost::asio::ip::make_address("127.0.0.1"), 7496);

  return std::make_shared<ibkr::Driver>(ibkrServer);
}