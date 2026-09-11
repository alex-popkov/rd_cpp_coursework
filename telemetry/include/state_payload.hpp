#pragma once
#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>

namespace telemetry {

struct StatePayload {
  std::uint32_t timestamp_ms;
  float x, y;
  float vx, vy;

  bool operator==(const StatePayload&) const = default;
};

// timestamp(4) + 4×float(4) = 20 байти на дроті.
constexpr std::size_t kStatePayloadSize = sizeof(std::uint32_t) + 4 * sizeof(float);

// Серіалізує payload у out (LE); повертає кількість записаних байтів (== kStatePayloadSize).
std::size_t serialize(const StatePayload& payload, std::span<std::uint8_t> out);

// Відновлює StatePayload з in; nullopt якщо байтів менше за kStatePayloadSize.
std::optional<StatePayload> deserialize(std::span<const std::uint8_t> in);

}  // namespace telemetry