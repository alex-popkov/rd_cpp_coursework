#include <gtest/gtest.h>
#include "cobs.hpp"

#include <cstdint>
#include <optional>
#include <vector>

using Bytes = std::vector<std::uint8_t>;

// Хелпер ховає роботу зі span/буфером: кодує вектор у вектор.
static Bytes encode(const Bytes& in)
{
  Bytes out(telemetry::cobs_encoded_max_size(in.size()));
  std::size_t n = telemetry::cobs_encode(in, out);
  out.resize(n);

  return out;
}

// Декод завжди не довший за вхід, тому in.size() — безпечний розмір буфера.
static std::optional<Bytes> decode(const Bytes& in)
{
  Bytes out(in.size());
  auto n = telemetry::cobs_decode(in, out);
  if (!n)
    return std::nullopt;
  out.resize(*n);

  return out;
}

TEST(Cobs, EncodesKnownVectors)
{
  EXPECT_EQ(encode({0x00}), (Bytes{0x01, 0x01}));
  EXPECT_EQ(encode({0x00, 0x00}), (Bytes{0x01, 0x01, 0x01}));
  EXPECT_EQ(encode({0x11, 0x22, 0x00, 0x33}), (Bytes{0x03, 0x11, 0x22, 0x02, 0x33}));
  EXPECT_EQ(encode({0x11, 0x22, 0x33, 0x44}), (Bytes{0x05, 0x11, 0x22, 0x33, 0x44}));
  EXPECT_EQ(encode({0x11, 0x00, 0x00, 0x00}), (Bytes{0x02, 0x11, 0x01, 0x01, 0x01}));
}

TEST(Cobs, EncodesEmptyInput)
{
  EXPECT_EQ(encode({}), (Bytes{0x01}));  // порожній блок = один code-байт
}

TEST(Cobs, DecodesKnownVector)
{
  EXPECT_EQ(decode({0x03, 0x11, 0x22, 0x02, 0x33}), (Bytes{0x11, 0x22, 0x00, 0x33}));
}

TEST(Cobs, RoundTrip)
{
  const Bytes samples[] = {
    {},
    {0x00},
    {0x00, 0x00},
    {0x01, 0x02, 0x03},
    {0x11, 0x00, 0x00, 0x00},
    {0xFF, 0x00, 0xAB},
  };
  for (const Bytes& s : samples) {
    EXPECT_EQ(decode(encode(s)), s);
  }
}

TEST(Cobs, RoundTripLongRunNoZeros)
{
  Bytes big(300, 0x41);  // 300 ненульових поспіль -> задіює гілку code == 0xFF
  EXPECT_EQ(decode(encode(big)), big);
}

TEST(Cobs, DecodeRejectsZeroInside)
{
  // 0x00 у COBS-блоці неможливий -> пошкодження.
  EXPECT_FALSE(decode({0x03, 0x11, 0x00}).has_value());
}

TEST(Cobs, DecodeRejectsTruncated)
{
  // code=5 обіцяє 4 байти далі, а їх лише 2 -> вихід за межі.
  EXPECT_FALSE(decode({0x05, 0x11, 0x22}).has_value());
}