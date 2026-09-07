#pragma once
#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>

namespace telemetry {

// Верхня межа розміру виходу для інпуту довжини n (для розміру out-буфера).
constexpr std::size_t cobs_encoded_max_size(std::size_t n)
{
  return n + n / 254 + 1;
}

// Кодує in у out, повертає кількість записаних байтів.
std::size_t cobs_encode(std::span<const std::uint8_t> in, std::span<std::uint8_t> out);

// Декодує in у out; std::nullopt якщо блок некоректний (є 0x00 всередині
// або code вказує за межі входу).
std::optional<std::size_t> cobs_decode(std::span<const std::uint8_t> in, std::span<std::uint8_t> out);

}  // namespace telemetry