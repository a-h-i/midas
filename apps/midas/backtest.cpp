#include "backtest/backtest.hpp"
#include "broker-interface/broker.hpp"
#include "broker-interface/instruments.hpp"
#include "broker-interface/order.hpp"
#include "data/data_stream.hpp"
#include "data/export.hpp"
#include "ibkr-driver/ibkr.hpp"
#include "trader/trader.hpp"
#include <boost/asio/ip/address.hpp>
#include <fstream>

using namespace midas::backtest::literals;

void backtestMomentumTrader() {
  midas::InstrumentEnum instrument = midas::InstrumentEnum::TSLA;
  auto traderFactory = [instrument](std::shared_ptr<midas::DataStream> streamPtr,
                          std::shared_ptr<midas::OrderManager> orderManager) {
    return midas::trader::momentumExploit(
        streamPtr, orderManager, instrument);
  };

  boost::asio::ip::tcp::endpoint ibkrServer(
      boost::asio::ip::make_address("127.0.0.1"), 7496);
  std::unique_ptr<midas::Broker> broker(new ibkr::Driver(ibkrServer));
  broker->connect();
  std::atomic_bool driverWorkerTermination{false};
  std::jthread brokerProcessor([&broker, &driverWorkerTermination] {
    while (!driverWorkerTermination.load()) {
      broker->processCycle();
    }
  });
  midas::backtest::BacktestResult results = midas::backtest::performBacktest(
      instrument, 10_days, traderFactory,
      *broker);
  std::cout << "Trade Summary"
            << "\nnumber entry orders: "
            << results.summary.numberOfEntryOrdersTriggered
            << "\nnumber of stop loss orders triggered "
            << results.summary.numberOfStopLossTriggered
            << "\nnumber of profit takers triggered "
            << results.summary.numberOfProfitTakersTriggered
            << "\nsuccess ratio " << results.summary.successRatio
            << "\nmax down turn " << results.summary.maxDownTurn
            << "\nmax up turn " << results.summary.maxUpTurn
            << "\nending balance " << results.summary.endingBalance
            << std::endl;
  std::ofstream sourceCsv("source.csv", std::ios::out),
      downSampledCsv("down_sampled.csv", std::ios::out),
      orderDetails("orders.txt", std::ios::out);
  sourceCsv << *results.originalStream;
  orderDetails << results.orderDetails;
  driverWorkerTermination.store(true);
}