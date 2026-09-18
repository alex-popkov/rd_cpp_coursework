#include "trajectory.hpp"
#include "state_payload.hpp"
#include "frame.hpp"
#include "frame_reader.hpp"
#include "kalman2d.hpp"
#include "channel_sim.hpp"

#include <cmath>
#include <cstdint>
#include <cstdio>
#include <optional>
#include <random>
#include <vector>

using namespace telemetry;

// Один прогін ланки при заданому рівні втрат; повертає RMS помилки позиції (м).
static double run_rms(double loss_prob, std::uint32_t seed)
{
  const int step_count = 400;
  const float dt = 0.05f, radius = 10.0f, omega = 0.5f;
  const float noise_pos = 0.20f, noise_vel = 0.10f;

  Kalman2D filter(1.0f, noise_pos, noise_vel);
  FrameReader reader;
  ChannelSim channel(loss_prob, 0.0, seed);
  std::mt19937 noise_rng(seed + 1000);
  std::normal_distribution<float> pos_noise_dist(0.0f, noise_pos);
  std::normal_distribution<float> vel_noise_dist(0.0f, noise_vel);

  bool initialized = false;
  double sq_sum = 0.0;  // Σ похибка²
  long count = 0;
  std::vector<std::uint8_t> payload(kStatePayloadSize);

  for (int step = 0; step < step_count; ++step) {
    float time_s = step * dt;
    TrueState true_state = circle_trajectory(time_s, radius, omega);

    float meas_x = true_state.x + pos_noise_dist(noise_rng);
    float meas_y = true_state.y + pos_noise_dist(noise_rng);
    float meas_vx = true_state.vx + vel_noise_dist(noise_rng);
    float meas_vy = true_state.vy + vel_noise_dist(noise_rng);

    StatePayload state_payload{static_cast<std::uint32_t>(time_s * 1000.0f), meas_x, meas_y, meas_vx, meas_vy};
    serialize(state_payload, payload);
    std::vector<std::uint8_t> wire(frame_encoded_max_size(payload.size()));
    std::size_t wire_len = frame_encode({FrameClass::State, static_cast<std::uint8_t>(step), 0, 0}, payload, wire);
    wire.resize(wire_len);

    std::vector<std::uint8_t> channel_out(wire.size());
    std::optional<std::size_t> delivered_len;
    if (step == 0) {
      channel_out = wire;
      delivered_len = wire.size();
    }
    else
      delivered_len = channel.transmit(wire, channel_out);

    std::optional<StatePayload> received;
    if (delivered_len) {
      channel_out.resize(*delivered_len);
      for (std::uint8_t byte : channel_out)
        if (auto parsed_frame = reader.push(byte); parsed_frame && parsed_frame->header.cls == FrameClass::State)
          received = deserialize(reader.payload());
    }

    if (!initialized) {
      if (received) {
        filter.update(received->x, received->y, received->vx, received->vy);
        initialized = true;
      }
    }
    else {
      filter.predict(dt);
      if (received)
        filter.update(received->x, received->y, received->vx, received->vy);
    }
    if (!initialized)
      continue;

    Estimate2D estimate = filter.estimate();
    double dx = estimate.x - true_state.x;
    double dy = estimate.y - true_state.y;
    sq_sum += dx * dx + dy * dy;  // квадрат відстані оцінка↔істина
    ++count;
  }
  return std::sqrt(sq_sum / count);  // корінь із середнього квадрата
}

int main()
{
  const int trials = 10;  // усереднення на кожен рівень для гладкості
  std::fprintf(stderr,
               "RMS помилки оцінки позиції (м) залежно від рівня втрат. "
               "Менше = краще; плавний ріст = телеметрія деградує плавно.\n");
  std::printf("loss,rms\n");
  for (int percent = 0; percent <= 8; ++percent) {
    double loss = percent * 0.1;
    double sum = 0.0;
    for (int trial = 0; trial < trials; ++trial)
      sum += run_rms(loss, static_cast<std::uint32_t>(trial));
    std::printf("%.1f,%.4f\n", loss, sum / trials);
  }
  return 0;
}