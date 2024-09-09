#include "ibkr-driver/ibkr.hpp"
#include "ibkr/internal/client.hpp"

void ibkr::test() {
  const auto address = boost::asio::ip::make_address("127.0.0.1");
  const auto endpoint = boost::asio::ip::tcp::endpoint(address, 7496);
  internal::Client client(endpoint);
}