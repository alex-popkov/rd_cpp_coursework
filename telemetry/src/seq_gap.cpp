#include "seq_gap.hpp"

namespace telemetry {

std::uint8_t SeqGapCounter::observe(std::uint8_t seq)
{
  std::uint8_t gap = 0;

  if (this->has_last_) {
    gap = static_cast<std::uint8_t>(seq - this->last_ - 1);
  }

  this->has_last_ = true;
  this->last_ = seq;

  return gap;
}

}  // namespace telemetry