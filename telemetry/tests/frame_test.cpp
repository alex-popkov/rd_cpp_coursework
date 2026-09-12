#include <gtest/gtest.h>
#include "frame.hpp"

#include <cstdint>
#include <vector>

using telemetry::frame_decode;
using telemetry::frame_encode;
using telemetry::FrameClass;
using telemetry::FrameHeader;

TEST(Frame, RoundTrip)
{
  FrameHeader hdr{FrameClass::State, 42, 7, 0x80};  // seq/ack/bitmap ненульові
  std::vector<std::uint8_t> payload = {0x11, 0x22, 0x33, 0x44};

  std::vector<std::uint8_t> wire(telemetry::frame_encoded_max_size(payload.size()));
  std::size_t n = frame_encode(hdr, payload, wire);
  ASSERT_GT(n, 0u);
  EXPECT_EQ(wire[n - 1], 0x00);  // роздільник в кінці

  std::vector<std::uint8_t> block(wire.begin(), wire.begin() + (n - 1));  // без роздільника
  std::vector<std::uint8_t> payload_out(telemetry::kMaxPayloadSize);
  auto parsed = frame_decode(block, payload_out);
  ASSERT_TRUE(parsed.has_value());
  EXPECT_EQ(parsed->header.cls, FrameClass::State);
  EXPECT_EQ(parsed->header.seq, 42);
  EXPECT_EQ(parsed->header.ack, 7);
  EXPECT_EQ(parsed->header.bitmap, 0x80);
  ASSERT_EQ(parsed->payload_len, payload.size());
  payload_out.resize(parsed->payload_len);
  EXPECT_EQ(payload_out, payload);
}

TEST(Frame, WireHasNoZeroInside)
{
  FrameHeader hdr{FrameClass::Command, 7, 0, 0};
  std::vector<std::uint8_t> payload = {0x00, 0x00, 0x00};  // навмисно нулі в даних
  std::vector<std::uint8_t> wire(telemetry::frame_encoded_max_size(payload.size()));
  std::size_t n = frame_encode(hdr, payload, wire);
  for (std::size_t i = 0; i + 1 < n; ++i) {
    EXPECT_NE(wire[i], 0x00);  // усередині кадру нулів нема
  }
  EXPECT_EQ(wire[n - 1], 0x00);
}

TEST(Frame, DecodeRejectsCrcError)
{
  FrameHeader hdr{FrameClass::State, 1, 0, 0};
  std::vector<std::uint8_t> payload = {0xAB, 0xCD};
  std::vector<std::uint8_t> wire(telemetry::frame_encoded_max_size(payload.size()));
  std::size_t n = frame_encode(hdr, payload, wire);

  std::vector<std::uint8_t> block(wire.begin(), wire.begin() + (n - 1));
  block[block.size() / 2] ^= 0xFF;  // псуємо один байт
  std::vector<std::uint8_t> payload_out(telemetry::kMaxPayloadSize);
  EXPECT_FALSE(frame_decode(block, payload_out).has_value());  // CRC або COBS ловить
}