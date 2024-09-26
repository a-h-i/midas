#include "broker-interface/order.hpp"
#include "trader/trader.hpp"
#include "ta_common.h"
/**
 * Very simple momentum trader
 * Intended to scalp MNQ and MES and similar indexes.
 */
class MomentumTrader : public midas::trader::Trader {
public:
  MomentumTrader(std::shared_ptr<midas::DataStream> source,
                 std::shared_ptr<midas::OrderManager> orderManager)
      : Trader(11, 120, source, orderManager) {}
  virtual ~MomentumTrader() {}
  virtual void decide() override {
    // do nothing
  }
};

std::unique_ptr<midas::trader::Trader> midas::trader::momentumExploit(
    std::shared_ptr<midas::DataStream> source,
    std::shared_ptr<midas::OrderManager> orderManager) {

  return std::make_unique<MomentumTrader>(source, orderManager);
}