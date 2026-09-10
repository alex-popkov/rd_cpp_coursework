#include <gtest/gtest.h>
#include "crc16.hpp"

#include <cstdint>
#include <vector>

TEST(Crc16, CanonicalCheckVector)
{
  const std::uint8_t msg[] = {'1', '2', '3', '4', '5', '6', '7', '8', '9'};
  EXPECT_EQ(telemetry::crc16_ccitt(msg), 0x29B1);  // стандарт CCITT-FALSE
}

TEST(Crc16, EmptyIsInit)
{
  std::vector<std::uint8_t> empty;
  EXPECT_EQ(telemetry::crc16_ccitt(empty), 0xFFFF);  // жодного байта -> лишається init
}

TEST(Crc16, DetectsSingleBitFlip)
{
  std::vector<std::uint8_t> a = {0x01, 0x02, 0x03, 0x04};
  std::vector<std::uint8_t> b = a;
  b[2] ^= 0x01;  // інвертуємо один біт
  EXPECT_NE(telemetry::crc16_ccitt(a), telemetry::crc16_ccitt(b));
}