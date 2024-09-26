#include "data/data_stream.hpp"
#include "exceptions/sampling_error.hpp"
#include "trader/trader.hpp"
#include <gtest/gtest.h>
#include <memory>

using namespace midas;

TEST(TraderData, TraderDataSampling) {
  const auto streamPtr = std::make_shared<DataStream>(5);
  
  trader::TraderData trader(11, 120, streamPtr);
}

TEST(TraderData, TraderDataInvalidSampleRate) {
  const auto streamPtr = std::make_shared<DataStream>(30);
  EXPECT_THROW(trader::TraderData(50, 64, streamPtr), SamplingError);
}