#include "trader/trader.hpp"
#include "broker-interface/order.hpp"

class MomentumTrader : public midas::trader::Trader {
private:
  const std::shared_ptr<midas::DataStream> source;

public:
  MomentumTrader(std::shared_ptr<midas::DataStream> &source)
      : source(source) {}
  virtual ~MomentumTrader() {}
};

std::unique_ptr<midas::trader::Trader> midas::trader::momentumExploit(
    std::shared_ptr<midas::DataStream> &source,
    std::shared_ptr<midas::OrderManager> orderManager) {

  return {};
}