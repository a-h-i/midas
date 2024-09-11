#include "ibkr-driver/ibkr.hpp"
#include "ibkr/internal/client.hpp"
#include <chrono>
#include <thread>
using namespace std::chrono_literals;
void ibkr::test() {
  auto logger = logging::create_channel_logger("test boy");
  const auto address = boost::asio::ip::make_address("127.0.0.1");
  const auto endpoint = boost::asio::ip::tcp::endpoint(address, 7496);
  internal::Client client(endpoint);
  client.addConnectListener(
      {"ID", [&logger] { INFO_LOG(logger) << "Connected"; }});
  client.connect();

  while (true) {
    std::this_thread::sleep_for(2s);
    client.processLoop();
  }
}