#include <gtest/gtest.h>
#include "channel_sim.hpp"

#include <cstdint>
#include <vector>

using telemetry::ChannelSim;

TEST(ChannelSim, LossZeroPassesAll)
{
  ChannelSim ch(0.0, 0.0, 1);
  std::vector<std::uint8_t> frame = {0x01, 0x02, 0x03};
  std::vector<std::uint8_t> out(frame.size());
  for (int i = 0; i < 100; ++i) {
    EXPECT_TRUE(ch.transmit(frame, out).has_value());
  }
}

TEST(ChannelSim, LossOneDropsAll)
{
  ChannelSim ch(1.0, 0.0, 1);
  std::vector<std::uint8_t> frame = {0x01, 0x02, 0x03};
  std::vector<std::uint8_t> out(frame.size());
  for (int i = 0; i < 100; ++i) {
    EXPECT_FALSE(ch.transmit(frame, out).has_value());
  }
}

TEST(ChannelSim, SameSeedSamePattern)
{
  ChannelSim a(0.5, 0.0, 42);
  ChannelSim b(0.5, 0.0, 42);
  std::vector<std::uint8_t> frame = {0xAA};
  std::vector<std::uint8_t> oa(1), ob(1);
  for (int i = 0; i < 200; ++i) {
    EXPECT_EQ(a.transmit(frame, oa).has_value(), b.transmit(frame, ob).has_value());  // однаковий патерн
  }
}

TEST(ChannelSim, BitFlipChangesBytes)
{
  ChannelSim ch(0.0, 1.0, 7);  // не губимо, завжди перевертаємо
  std::vector<std::uint8_t> frame = {0x11, 0x22, 0x33, 0x44};
  std::vector<std::uint8_t> out(frame.size());
  auto n = ch.transmit(frame, out);
  ASSERT_TRUE(n.has_value());
  ASSERT_EQ(*n, frame.size());
  out.resize(*n);
  EXPECT_NE(out, frame);  // один біт перевернуто -> байти інші
}