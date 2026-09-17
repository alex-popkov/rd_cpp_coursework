#pragma once
#include <deque>
#include <vector>
#include "hal.hpp"

namespace telemetry {

// Мок для тестів
class MockHal : public IHal {
public:
  std::size_t read_bytes(std::span<std::uint8_t> buf) override;
  std::size_t write_bytes(std::span<const std::uint8_t> data) override;
  std::uint64_t now_us() const override { return clock_us_; }

  // керування з тесту
  void feed(std::span<const std::uint8_t> bytes);  // підкинути вхідні байти
  const std::vector<std::uint8_t>& sent() const { return sent_; }
  void set_now_us(std::uint64_t t) { clock_us_ = t; }

private:
  std::deque<std::uint8_t> in_;
  std::vector<std::uint8_t> sent_;
  std::uint64_t clock_us_ = 0;
};

}  // namespace telemetry