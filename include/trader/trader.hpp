#pragma once
#include "data/data_stream.hpp"
#include <memory>

namespace midas::trader {

  struct Trader {
    virtual void decide(const DataStream &data) = 0;
  };

  /**
   * Designed for fast 2 min momentum exploitation
   */
  std::unique_ptr<Trader> momentumExploit();
}