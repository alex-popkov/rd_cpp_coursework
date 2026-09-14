#pragma once
#include <cstdint>

namespace telemetry {

// Рахує пропущені вимірювання за розривом у SEQ (простір 8 біт, mod 256).
class SeqGapCounter {
public:
  // Кількість пропущених між попереднім і цим кадром.
  // Перший виклик лише встановлює базу і повертає 0.
  std::uint8_t observe(std::uint8_t seq);

private:
  bool has_last_ = false;
  std::uint8_t last_ = 0;
};

}  // namespace telemetry