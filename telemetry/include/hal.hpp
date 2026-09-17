#pragma once
#include <cstddef>
#include <cstdint>
#include <span>

namespace telemetry {

class IHal {
public:
  virtual ~IHal() = default;
  virtual std::size_t read_bytes(std::span<std::uint8_t> buf) = 0;          // скільки прочитано (0 якщо нема)
  virtual std::size_t write_bytes(std::span<const std::uint8_t> data) = 0;  // скільки записано
  virtual std::uint64_t now_us() const = 0;                                 // монотонний час, мкс
};

}  // namespace telemetry