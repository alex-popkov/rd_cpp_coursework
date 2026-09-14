#include "channel_sim.hpp"

namespace telemetry {

ChannelSim::ChannelSim(double loss_prob, double bitflip_prob, std::uint32_t seed)
  : loss_prob_(loss_prob)
  , bitflip_prob_(bitflip_prob)
  , rng_(seed)
{
}

std::optional<std::size_t> ChannelSim::transmit(std::span<const std::uint8_t> frame, std::span<std::uint8_t> out)
{
  std::uniform_real_distribution<double> unit(0.0, 1.0);
  if (unit(rng_) < loss_prob_) {
    return std::nullopt;
  }

  for (std::size_t i = 0; i < frame.size(); ++i) {
    out[i] = frame[i];
  }

  if (frame.size() > 0 && unit(rng_) < bitflip_prob_) {
    std::uniform_int_distribution<std::size_t> byte_pick(0, frame.size() - 1);
    std::uniform_int_distribution<int> bit_pick(0, 7);
    std::size_t bi = byte_pick(rng_);
    int b = bit_pick(rng_);
    out[bi] ^= static_cast<std::uint8_t>(1u << b);
  }

  return frame.size();
};

}  // namespace telemetry