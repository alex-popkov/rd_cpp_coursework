#include "crc16.hpp"

namespace telemetry {

std::uint16_t crc16_ccitt(std::span<const std::uint8_t> data)
{
  std::uint16_t crc = 0xFFFF;  // init
  for (std::uint8_t byte : data) {
    crc ^= static_cast<std::uint16_t>(byte) << 8;  // байт у старший байт CRC (MSB-first)
    for (int bit = 0; bit < 8; ++bit) {
      if (crc & 0x8000) {
        crc = static_cast<std::uint16_t>((crc << 1) ^ 0x1021);
      }
      else {
        crc = static_cast<std::uint16_t>(crc << 1);
      }
    }
  }
  return crc;  // xorout=0x0000 -> без фінального XOR
}

}  // namespace telemetry