#include <gtest/gtest.h>
#include "state_payload.hpp"

#include <cstdint>
#include <vector>

using telemetry::StatePayload;

TEST(StatePayload, SizeIs20)
{
  EXPECT_EQ(telemetry::kStatePayloadSize, 20u);
}

TEST(StatePayload, RoundTrip)
{
  StatePayload p{123456, 1.5f, -2.25f, 0.0f, 3.75f};
  std::vector<std::uint8_t> buf(telemetry::kStatePayloadSize);
  std::size_t n = telemetry::serialize(p, buf);
  ASSERT_EQ(n, telemetry::kStatePayloadSize);
  auto back = telemetry::deserialize(buf);
  ASSERT_TRUE(back.has_value());
  EXPECT_EQ(*back, p);
}

TEST(StatePayload, TimestampIsLittleEndian)
{
  StatePayload p{0x01020304, 0, 0, 0, 0};
  std::vector<std::uint8_t> buf(telemetry::kStatePayloadSize);
  telemetry::serialize(p, buf);
  EXPECT_EQ(buf[0], 0x04);  // молодший байт перший
  EXPECT_EQ(buf[1], 0x03);
  EXPECT_EQ(buf[2], 0x02);
  EXPECT_EQ(buf[3], 0x01);
}

TEST(StatePayload, DeserializeRejectsShort)
{
  std::vector<std::uint8_t> tooShort(telemetry::kStatePayloadSize - 1);
  EXPECT_FALSE(telemetry::deserialize(tooShort).has_value());
}