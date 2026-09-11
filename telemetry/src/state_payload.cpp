#include <bit>
#include "state_payload.hpp"

namespace telemetry {

// Кладе uint32 у out[off..off+3] у порядку LE, повертає новий зсув.
std::size_t put_u32(std::span<std::uint8_t> out, std::size_t off, std::uint32_t value)
{
  out[off + 0] = static_cast<std::uint8_t>(value & 0xFF);  // молодший
  out[off + 1] = static_cast<std::uint8_t>((value >> 8) & 0xFF);
  out[off + 2] = static_cast<std::uint8_t>((value >> 16) & 0xFF);
  out[off + 3] = static_cast<std::uint8_t>((value >> 24) & 0xFF);  // старший
  return off + 4;
}

// Кладе float (32) у out[off..off+3] у порядку LE, повертає новий зсув.
std::size_t put_f32(std::span<std::uint8_t> out, std::size_t off, float value)
{
  return put_u32(out, off, std::bit_cast<std::uint32_t>(value));
}

std::uint32_t get_u32(std::span<const std::uint8_t> in, std::size_t off)
{
  return std::uint32_t(in[off + 0])                                                  // молодший — на місце 0
         | (std::uint32_t(in[off + 1]) << 8)                                         // наступний — на біти 8..15
         | (std::uint32_t(in[off + 2]) << 16) | (std::uint32_t(in[off + 3]) << 24);  // старший — на біти 24..31
}

float get_f32(std::span<const std::uint8_t> in, std::size_t off)
{
  return std::bit_cast<float>(get_u32(in, off));  // u32 -> назад у біти float
}

// Серіалізує payload у out (LE); повертає кількість записаних байтів (== kStatePayloadSize).
std::size_t serialize(const StatePayload& payload, std::span<std::uint8_t> out)
{
  std::size_t off = 0;
  off = put_u32(out, off, payload.timestamp_ms);
  off = put_f32(out, off, payload.x);
  off = put_f32(out, off, payload.y);
  off = put_f32(out, off, payload.vx);
  off = put_f32(out, off, payload.vy);

  return off;
};

// Відновлює StatePayload з in; nullopt якщо байтів менше за kStatePayloadSize.
std::optional<StatePayload> deserialize(std::span<const std::uint8_t> in)
{
  if (in.size() < kStatePayloadSize) {
    return std::nullopt;
  }
  StatePayload p{get_u32(in, 0), get_f32(in, 4), get_f32(in, 8), get_f32(in, 12), get_f32(in, 16)};

  return p;
};

}  // namespace telemetry