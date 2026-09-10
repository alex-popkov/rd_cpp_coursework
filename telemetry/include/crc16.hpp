#pragma once
#include <cstdint>
#include <span>

namespace telemetry {

// CRC-16/CCITT-FALSE: poly=0x1021, init=0xFFFF, refin/refout=false, xorout=0x0000.
// Контрольне значення для "123456789" == 0x29B1.
std::uint16_t crc16_ccitt(std::span<const std::uint8_t> data);

}  // namespace telemetry