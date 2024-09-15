#include "ibkr-driver/ibkr.hpp"
#include "ibkr/internal/build_contracts.hpp"
#include "ibkr/internal/client.hpp"
#include "midas/instruments.hpp"
#include "ibkr-driver/subscription.hpp"
#include <chrono>
#include <thread>

using namespace std::chrono_literals;
void ibkr::test() {
  auto logger = logging::create_channel_logger("test boy");
  const auto address = boost::asio::ip::make_address("127.0.0.1");
  const auto endpoint = boost::asio::ip::tcp::endpoint(address, 7496);
  internal::Client client(endpoint);
  std::shared_ptr<Subscription> sub = std::make_shared<Subscription>(Symbols::MNQ, false);
  sub->barListeners.add_listener([&logger]([[maybe_unused]]const Subscription &sub, midas::Bar bar) {
    INFO_LOG(logger) << "Received bar: " << bar;
  });
  sub->endListeners.add_listener([&logger] ([[maybe_unused]] const Subscription &sub) {
    INFO_LOG(logger) << "Received end event";
  });

  client.addConnectListener(
     [&logger, &client, &sub] { 
        INFO_LOG(logger) << "Connected"; 
        client.addSubscription(sub);
      });
  client.connect();

  while (true) {
    client.processCycle();
    std::this_thread::sleep_for(2s);
  }
}