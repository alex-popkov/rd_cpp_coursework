#include "state_receiver.hpp"
#include "state_payload.hpp"
#include "frame.hpp"

namespace telemetry {

StateReceiver::StateReceiver(float sigma_a, float sigma_pos, float sigma_vel)
  : filter_(sigma_a, sigma_pos, sigma_vel)
{
}

std::optional<Estimate2D> StateReceiver::push(std::uint8_t byte)
{
  auto frame = reader_.push(byte);
  if (!frame || frame->header.cls != FrameClass::State) {
    return std::nullopt;
  }

  auto state_payload = deserialize(reader_.payload());
  if (!state_payload) {
    return std::nullopt;
  }

  if (!initialized_) {
    filter_.update(state_payload->x, state_payload->y, state_payload->vx, state_payload->vy);
    last_ts_ = state_payload->timestamp_ms;
    initialized_ = true;

    return filter_.estimate();
  }

  if (state_payload->timestamp_ms <= last_ts_) {
    return std::nullopt;
  }

  float dt = (state_payload->timestamp_ms - last_ts_) / 1000.0f;
  filter_.predict(dt);
  filter_.update(state_payload->x, state_payload->y, state_payload->vx, state_payload->vy);
  last_ts_ = state_payload->timestamp_ms;

  return filter_.estimate();
};

Estimate2D StateReceiver::estimate() const
{
  return filter_.estimate();
}

}  // namespace telemetry