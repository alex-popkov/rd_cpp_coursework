#include "mock_hal.hpp"
#include <algorithm>

namespace telemetry {

std::size_t MockHal::read_bytes(std::span<std::uint8_t> buf)
{
  std::size_t n = std::min(buf.size(), in_.size());

  for (std::size_t i = 0; i < n; ++i) {
    buf[i] = in_.front();
    in_.pop_front();
  }

  return n;
}

std::size_t MockHal::write_bytes(std::span<const std::uint8_t> data)
{
  for (std::size_t i = 0; i < data.size(); ++i) {
    sent_.push_back(data[i]);
  }

  return data.size();
}

void MockHal::feed(std::span<const std::uint8_t> bytes)
{
  in_.insert(in_.end(), bytes.begin(), bytes.end());
}

}  // namespace telemetry