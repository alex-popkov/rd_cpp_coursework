#include <gtest/gtest.h>
#include "mock_hal.hpp"
#include <vector>

using telemetry::MockHal;

TEST(MockHal, WriteCaptures)
{
  MockHal hal;
  std::vector<std::uint8_t> data = {1, 2, 3};
  EXPECT_EQ(hal.write_bytes(data), 3u);
  EXPECT_EQ(hal.sent(), (std::vector<std::uint8_t>{1, 2, 3}));
}

TEST(MockHal, ReadReturnsFed)
{
  MockHal hal;
  std::vector<std::uint8_t> in = {9, 8, 7};
  hal.feed(in);
  std::vector<std::uint8_t> buf(10);
  std::size_t n = hal.read_bytes(buf);
  ASSERT_EQ(n, 3u);
  buf.resize(n);
  EXPECT_EQ(buf, in);
}

TEST(MockHal, ReadRespectsBufferSize)
{
  MockHal hal;
  hal.feed(std::vector<std::uint8_t>{1, 2, 3, 4, 5});
  std::vector<std::uint8_t> buf(2);
  EXPECT_EQ(hal.read_bytes(buf), 2u);
  EXPECT_EQ(hal.read_bytes(buf), 2u);
  EXPECT_EQ(hal.read_bytes(buf), 1u);
  EXPECT_EQ(hal.read_bytes(buf), 0u);  // порожньо
}

TEST(MockHal, ClockControllable)
{
  MockHal hal;
  hal.set_now_us(12345);
  EXPECT_EQ(hal.now_us(), 12345u);
}