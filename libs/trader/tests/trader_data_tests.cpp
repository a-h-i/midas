#include "data/data_stream.hpp"
#include "exceptions/sampling_error.hpp"
#include "trader/trader.hpp"
#include <gtest/gtest.h>
#include <memory>
using namespace std::chrono_literals;

using namespace midas;

TEST(TraderData, TraderDataSampling) {
  const int streamBarSeconds = 5;
  const auto streamPtr = std::make_shared<DataStream>(streamBarSeconds);
  int streamBarsAdded = 0;
  const int traderCandleSeconds = 120;
  const int lookBack = 11;
  const int traderToStreamBarRatio = traderCandleSeconds / streamBarSeconds;
  //  Enough for down sampling
  const int barsPerLookBack = lookBack * traderCandleSeconds;

  for (; streamBarsAdded < 13; streamBarsAdded++) {
    Bar bar(5, 10, 300, 100, 150, 250, 400, 1000,
            boost::posix_time::second_clock::universal_time());
    streamPtr->addBars(bar);
  }
  streamPtr->waitForData(200ms);
  trader::TraderData trader(lookBack, traderCandleSeconds, streamPtr);
  trader.processSource(); // first time only, other wise waitForData calls handlers
  EXPECT_FALSE(trader.ok());
  EXPECT_EQ(trader.size(), 0);
  for (; streamBarsAdded < traderToStreamBarRatio; streamBarsAdded++) {
    Bar bar(5, 10, 300, 100, 150, 250, 400, 1000,
            boost::posix_time::second_clock::universal_time());
    streamPtr->addBars(bar);
  }
  streamPtr->waitForData(200ms);
  EXPECT_FALSE(trader.ok());
  EXPECT_EQ(trader.size(), 1);

  for (; streamBarsAdded < barsPerLookBack;
       streamBarsAdded += traderToStreamBarRatio) {
    std::vector<Bar> bars(
        traderToStreamBarRatio,
        Bar(5, 10, 300, 100, 150, 250, 400, 1000,
            boost::posix_time::second_clock::universal_time()));
    streamPtr->addBars(bars.begin(), bars.end());
  }
  streamPtr->waitForData(200ms);
  EXPECT_TRUE(trader.ok()); // enough to meet look back capacity
  EXPECT_EQ(trader.size(), lookBack);
}

TEST(TraderData, TraderDataInvalidSampleRate) {
  const auto streamPtr = std::make_shared<DataStream>(30);
  EXPECT_THROW(trader::TraderData(50, 64, streamPtr), SamplingError);
}


TEST(TraderData, ContinuousDataFeed) {
  const int streamBarSeconds = 5;
  const auto streamPtr = std::make_shared<DataStream>(streamBarSeconds);
  trader::TraderData trader(10, 60, streamPtr);

  // Feed 20 bars in two batches, enough for 2 full downsampled candles
  for (int i = 0; i < 24; ++i) {
      streamPtr->addBars(Bar(5, 10 + i, 8 + i, 9 + i, 10 + i, 11 + i, 50 + i, 100 + i,
            boost::posix_time::second_clock::universal_time()));
  }
  streamPtr->waitForData(0ms);
  trader.processSource();

  // Check the size after processing
  EXPECT_EQ(trader.size(), 2);  // Expect 2 downsampled bars

  // Now add more bars and validate size increments as expected
  for (int i = 0; i < 240; ++i) {
      streamPtr->addBars(Bar(5, 12 + i, 7 + i, 8 + i, 9 + i, 10 + i, 40 + i, 90 + i,
            boost::posix_time::second_clock::universal_time()));
  }
  streamPtr->waitForData(0ms);
  trader.processSource();

  EXPECT_EQ(trader.size(), 10);  // Should fill up to the lookBack limit
  EXPECT_TRUE(trader.ok());      // Check that the lookback capacity is met
}