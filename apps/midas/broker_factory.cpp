#include "broker/broker_factory.hpp"
#include "broker-interface/broker.hpp"
#include "ibkr-driver/ibkr.hpp"

std::shared_ptr<midas::Broker> createIBKRBroker() {
  boost::asio::ip::tcp::endpoint ibkrServer(
      boost::asio::ip::make_address("127.0.0.1"), 7496);

  return std::make_shared<ibkr::Driver>(ibkrServer);
}