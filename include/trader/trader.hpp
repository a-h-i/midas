#pragma once
#include "broker-interface/instruments.hpp"
#include "broker-interface/order.hpp"

#include "data/data_stream.hpp"

#include "base_trader.hpp"
#include <boost/signals2/variadic_signal.hpp>
#include <memory>
namespace midas::trader {


enum class TraderType {
  MomentumTrader,
  MACDTrader,
  MeanReversionTrader,
};


std::unique_ptr<Trader>
momentumExploit(std::shared_ptr<DataStream> source,
                std::shared_ptr<midas::OrderManager> orderManager,
                InstrumentEnum instrument, std::size_t entryQuantity);
std::unique_ptr<Trader> meanReversion(std::shared_ptr<DataStream> source,
                std::shared_ptr<midas::OrderManager> orderManager,
                InstrumentEnum instrument, std::size_t entryQuantity);

std::unique_ptr<Trader> macdExploit(std::shared_ptr<DataStream> source,
                std::shared_ptr<midas::OrderManager> orderManager,
                InstrumentEnum instrument, std::size_t entryQuantity);

std::unique_ptr<Trader> createTrader(TraderType type, std::shared_ptr<DataStream> source,
                std::shared_ptr<midas::OrderManager> orderManager,
                InstrumentEnum instrument, std::size_t entryQuantity);
} // namespace midas::trader