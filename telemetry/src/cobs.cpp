#include "cobs.hpp"

namespace telemetry {

std::size_t cobs_encode(std::span<const std::uint8_t> in, std::span<std::uint8_t> out)
{
  std::uint8_t code_when_zero = 1;
  std::size_t code_pos = 0;
  std::size_t out_i = 1;

  for (std::size_t i = 0; i < in.size(); ++i) {
    if (in[i] == 0x00) {
      out[code_pos] = code_when_zero;
      code_pos = out_i;
      code_when_zero = 1;
    }
    else {
      out[out_i] = in[i];
      code_when_zero++;
    }
    out_i++;

    if (code_when_zero == 0xFF) {
      out[code_pos] = code_when_zero;
      code_pos = out_i;
      out_i++;
      code_when_zero = 1;
    }
  }
  out[code_pos] = code_when_zero;

  return out_i;
}

std::optional<std::size_t> cobs_decode(std::span<const std::uint8_t> in, std::span<std::uint8_t> out)
{
  std::size_t out_i = 0;
  std::size_t index = 0;

  while (index < in.size()) {
    const std::size_t n = in[index];
    index++;
    if (n == 0x00) {
      return std::nullopt;
    }
    for (std::size_t k = 0; k < n - 1; ++k) {
      if (index >= in.size() || in[index] == 0x00) {
        return std::nullopt;
      }
      out[out_i] = in[index];
      out_i++;
      index++;
    }
    if (n != 0xFF && index < in.size()) {
      out[out_i] = 0x00;
      out_i++;
    }
  }

  return out_i;
}

}  // namespace telemetry