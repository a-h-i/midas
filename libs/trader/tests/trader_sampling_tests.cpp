#include "data/bar.hpp"
#include "data/data_stream.hpp"
#include "trader/trader.hpp"
#include "gtest/gtest.h"
using midas::Bar;
using namespace std::chrono_literals;
// Set up mock data source
class MockDataStream : public midas::DataStream {
public:
  MockDataStream(std::size_t barSize) : midas::DataStream(barSize) {}

  void addBar(double open, double high, double low, double close,
              double volume) {
    opens.push_back(open);
    highs.push_back(high);
    lows.push_back(low);
    closes.push_back(close);
    volumes.push_back(volume);
    tradeCounts.push_back(1); // Adding a trade count for simplicity
    double wap = (high + low + close) / 3.0;
    waps.push_back(wap);
  }
};

class TraderDataTest : public ::testing::Test {
protected:
  std::shared_ptr<MockDataStream> dataStream;
  std::unique_ptr<midas::trader::TraderData> traderData;

  void SetUp() override {
    dataStream =
        std::make_shared<MockDataStream>(30); // Assuming 30-second bars
    traderData = std::make_unique<midas::trader::TraderData>(
        10, 60, dataStream); // Downsampling to 1 minute
  }
};

TEST_F(TraderDataTest, BasicDownsampling) {
  // Add 4 bars, expected 2 downsampled bars
  dataStream->addBar(1.0, 2.0, 0.5, 1.5, 100.0);
  dataStream->addBar(1.5, 2.5, 1.0, 2.0, 150.0);
  dataStream->addBar(2.0, 3.0, 1.5, 2.5, 200.0);
  dataStream->addBar(2.5, 3.5, 2.0, 3.0, 250.0);

  traderData->processSource();

  EXPECT_EQ(traderData->size(), 2);
  std::vector<unsigned int> trades;
  std::vector<double> closes, volumes, highs, lows, opens, vwaps;
  traderData->copy(trades, highs, lows, opens, closes, vwaps, volumes);
  EXPECT_EQ(opens[0], 1.0);  // First open
  EXPECT_EQ(closes[1], 3.0); // Last close of second bar
  EXPECT_EQ(highs[0], 2.5);  // First high
  EXPECT_EQ(lows[0], 0.5);   // First low
}
TEST(VWAPTests, VWAPCalculation) {
  const int streamBarSeconds = 5;
  const auto streamPtr = std::make_shared<midas::DataStream>(5);
  midas::trader::TraderData trader(10, 60,
                                   streamPtr); // Downsample to 1-minute candles

  // Adding 12 bars to form a full 1-minute candle (each bar is 5 seconds)
  streamPtr->addBars(
      midas::Bar(5, 10, 310, 150, 160, 290, 220, 1000,
                 boost::posix_time::second_clock::universal_time()));
  streamPtr->addBars(
      midas::Bar(5, 10, 315, 155, 165, 295, 230, 1200,
                 boost::posix_time::second_clock::universal_time()));
  streamPtr->addBars(
      midas::Bar(5, 10, 320, 160, 170, 300, 240, 1100,
                 boost::posix_time::second_clock::universal_time()));
  streamPtr->addBars(
      midas::Bar(5, 10, 325, 155, 175, 305, 250, 900,
                 boost::posix_time::second_clock::universal_time()));
  streamPtr->addBars(
      midas::Bar(5, 10, 330, 160, 180, 310, 260, 1300,
                 boost::posix_time::second_clock::universal_time()));
  streamPtr->addBars(
      midas::Bar(5, 10, 335, 165, 185, 315, 270, 800,
                 boost::posix_time::second_clock::universal_time()));
  streamPtr->addBars(
      midas::Bar(5, 10, 340, 170, 190, 320, 280, 1200,
                 boost::posix_time::second_clock::universal_time()));
  streamPtr->addBars(
      midas::Bar(5, 10, 345, 175, 195, 325, 290, 1500,
                 boost::posix_time::second_clock::universal_time()));
  streamPtr->addBars(
      midas::Bar(5, 10, 350, 180, 200, 330, 300, 1400,
                 boost::posix_time::second_clock::universal_time()));
  streamPtr->addBars(
      midas::Bar(5, 10, 355, 185, 205, 335, 310, 1600,
                 boost::posix_time::second_clock::universal_time()));
  streamPtr->addBars(
      midas::Bar(5, 10, 360, 190, 210, 340, 320, 1100,
                 boost::posix_time::second_clock::universal_time()));
  streamPtr->addBars(
      midas::Bar(5, 10, 365, 195, 215, 345, 330, 1000,
                 boost::posix_time::second_clock::universal_time()));

  // Wait for data to be processed and generate the downsampled 1-minute bar
  streamPtr->waitForData(0ms);
  trader.processSource();

  // Expected VWAP calculation for one 1-minute candle
  double expectedVWAP =
      ((220.0 * 1000) + (230 * 1200) + (240 * 1100) + (250 * 900) +
       (260 * 1300) + (270 * 800) + (280 * 1200) + (290 * 1500) + (300 * 1400) +
       (310 * 1600) + (320 * 1100) + (330 * 1000)) /
      (1000 + 1200 + 1100 + 900 + 1300 + 800 + 1200 + 1500 + 1400 + 1600 +
       1100 + 1000);

  EXPECT_EQ(trader.size(), 1); // We expect one downsampled bar
  std::vector<unsigned int> trades;
  std::vector<double> closes, volumes, highs, lows, opens, vwaps;
  trader.copy(trades, highs, lows, opens, closes, vwaps, volumes);
  EXPECT_NEAR(vwaps.back(), expectedVWAP, 0.01);
}

TEST(VWAPTests, ZeroVolumeVWAP) {
  const int streamBarSeconds = 5;
  const auto streamPtr = std::make_shared<midas::DataStream>(streamBarSeconds);
  midas::trader::TraderData trader(10, 60,
                                   streamPtr); // Downsample to 1-minute candles

  // Adding 12 bars with zero volume
  std::vector<midas::Bar> zeroVolumeBars(
      12, midas::Bar(5, 10, 300, 100, 150, 250, 200, 0, // volume is zero
                     boost::posix_time::second_clock::universal_time()));

  for (const auto &bar : zeroVolumeBars) {
    streamPtr->addBars(bar);
  }

  streamPtr->waitForData(200ms);
  trader.processSource();

  EXPECT_EQ(trader.size(), 1); // One downsampled bar expected
  std::vector<unsigned int> trades;
  std::vector<double> closes, volumes, highs, lows, opens, vwaps;
  trader.copy(trades, highs, lows, opens, closes, vwaps, volumes);
  EXPECT_EQ(vwaps.back(), 0.0); // VWAP should be zero due to zero volume
}