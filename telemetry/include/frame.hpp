#pragma once
#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>

#include "cobs.hpp"

// frame
//  [LEN][CLASS][SEQ][ACK][BITMAP][ payload×LEN ][ CRC:2 ]   ← далі все це через COBS + 0x00
//   └──────── CRC16 над цим діапазоном ────────┘
namespace telemetry {

enum class FrameClass : std::uint8_t {
  State = 0,
  Command = 1,
};

// Логічний заголовок. LEN на дроті виводиться з розміру payload, тому тут його нема.
struct FrameHeader {
  FrameClass cls;
  std::uint8_t seq;
  std::uint8_t ack = 0;     // piggyback ACK, reserved до Кроку CMD
  std::uint8_t bitmap = 0;  // SACK-lite, reserved до Кроку CMD
};

constexpr std::size_t kFrameHeaderSize = 5;  // LEN+CLASS+SEQ+ACK+BITMAP на дроті
constexpr std::size_t kFrameCrcSize = 2;
constexpr std::size_t kMaxPayloadSize = 255;  // LEN — один байт

struct ParsedFrame {
  FrameHeader header;
  std::size_t payload_len;  // скільки байтів лягло в payload_out
};

// Верхня межа розміру дротового кадру для payload довжини n.
constexpr std::size_t frame_encoded_max_size(std::size_t payload_len)
{
  return cobs_encoded_max_size(kFrameHeaderSize + payload_len + kFrameCrcSize) + 1;  // +1 роздільник
}

// Будує повний дротовий кадр: COBS(вміст) + 0x00. Повертає кількість байтів у out.
// додає заголовок, CRC, COBS, роздільник
std::size_t frame_encode(const FrameHeader& header, std::span<const std::uint8_t> payload, std::span<std::uint8_t> out);

// Розбирає ОДИН COBS-блок (байти кадру БЕЗ роздільника): COBS-decode -> перевірка LEN -> CRC.
// payload_out отримує байти payload. nullopt на будь-якій відмові.
std::optional<ParsedFrame> frame_decode(std::span<const std::uint8_t> block, std::span<std::uint8_t> payload_out);

}  // namespace telemetry