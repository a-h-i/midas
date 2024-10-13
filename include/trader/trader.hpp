#pragma once
#include "broker-interface/instruments.hpp"
#include "broker-interface/order.hpp"

#include "data/data_stream.hpp"

#include "base_trader.hpp"
#include <boost/signals2/variadic_signal.hpp>
#include <memory>
namespace midas::trader {

/**
 * Designed for fast 2 min momentum exploitation
 */
std::unique_ptr<Trader>
momentumExploit(std::shared_ptr<DataStream> source,
                std::shared_ptr<midas::OrderManager> orderManager,
                InstrumentEnum instrument);
} // namespace midas::trader