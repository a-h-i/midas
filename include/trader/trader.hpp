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

inline std::string to_string(TraderType type) {
  switch (type) {
  case TraderType::MomentumTrader:
    return "MomentumTrader";
  case TraderType::MACDTrader:
    return "MACDTrader";
  case TraderType::MeanReversionTrader:
    return "MeanReversionTrader";
  }
  throw std::runtime_error("Invalid TraderType");
}
inline TraderType trader_from_string(const std::string &str) {
  if (str == "MomentumTrader")
    return TraderType::MomentumTrader;
  if (str == "MACDTrader")
    return TraderType::MACDTrader;
  if (str == "MeanReversionTrader")
    return TraderType::MeanReversionTrader;
  throw std::runtime_error("Invalid TraderType");
}

std::unique_ptr<Trader>
momentumExploit(std::shared_ptr<DataStream> source,
                std::shared_ptr<midas::OrderManager> orderManager,
                InstrumentEnum instrument, std::size_t entryQuantity);
std::unique_ptr<Trader>
meanReversion(std::shared_ptr<DataStream> source,
              std::shared_ptr<midas::OrderManager> orderManager,
              InstrumentEnum instrument, std::size_t entryQuantity);

std::unique_ptr<Trader>
macdExploit(std::shared_ptr<DataStream> source,
            std::shared_ptr<midas::OrderManager> orderManager,
            InstrumentEnum instrument, std::size_t entryQuantity);

std::unique_ptr<Trader>
createTrader(TraderType type, std::shared_ptr<DataStream> source,
             std::shared_ptr<midas::OrderManager> orderManager,
             InstrumentEnum instrument, std::size_t entryQuantity);
} // namespace midas::trader