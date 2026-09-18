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

int main()
{
  // --- параметри ---
  const int step_count = 400;         // кроків
  const float dt = 0.05f;             // 20 Гц
  const float radius = 10.0f;         // радіус кола
  const float omega = 0.5f;           // кутова швидкість, рад/с
  const double loss_prob = 0.40;      // 40% втрат
  const double bitflip_prob = 0.0;    // інверсія бітів (0 = вимкнено)
  const float noise_pos = 0.20f;      // шум вимірювання позиції
  const float noise_vel = 0.10f;      // шум вимірювання швидкості
  const float blackout_start = 8.0f;  // повний блекаут: початок, с
  const float blackout_end = 10.0f;   // повний блекаут: кінець, с

  Kalman2D filter(/*sigma_a=*/1.0f, noise_pos, noise_vel);
  FrameReader reader;
  ChannelSim channel(loss_prob, bitflip_prob, /*seed=*/42);
  std::mt19937 noise_rng(7);
  std::normal_distribution<float> pos_noise_dist(0.0f, noise_pos);
  std::normal_distribution<float> vel_noise_dist(0.0f, noise_vel);

  std::printf("t,true_x,true_y,meas_x,meas_y,est_x,est_y,sig_x,sig_y\n");

  bool initialized = false;
  std::vector<std::uint8_t> payload(kStatePayloadSize);

  for (int step = 0; step < step_count; ++step) {
    float time_s = step * dt;
    TrueState true_state = circle_trajectory(time_s, radius, omega);

    // вимірювання = істина + гаусів шум
    float meas_x = true_state.x + pos_noise_dist(noise_rng);
    float meas_y = true_state.y + pos_noise_dist(noise_rng);
    float meas_vx = true_state.vx + vel_noise_dist(noise_rng);
    float meas_vy = true_state.vy + vel_noise_dist(noise_rng);

    // кадр STATE
    StatePayload state_payload{static_cast<std::uint32_t>(time_s * 1000.0f), meas_x, meas_y, meas_vx, meas_vy};
    serialize(state_payload, payload);
    std::vector<std::uint8_t> wire(frame_encoded_max_size(payload.size()));
    std::size_t wire_len = frame_encode({FrameClass::State, static_cast<std::uint8_t>(step), 0, 0}, payload, wire);
    wire.resize(wire_len);

    // канал (перший кадр завжди пропускаємо, щоб фільтр стартував)
    bool blackout = (time_s >= blackout_start && time_s < blackout_end);
    std::vector<std::uint8_t> channel_out(wire.size());
    std::optional<std::size_t> delivered_len;
    if (step == 0) {
      channel_out = wire;
      delivered_len = wire.size();
    }
    else if (blackout) {
      delivered_len = std::nullopt;  // повний блекаут -> жодного кадру
    }
    else
      delivered_len = channel.transmit(wire, channel_out);

    // приймання
    std::optional<StatePayload> received;
    if (delivered_len) {
      channel_out.resize(*delivered_len);
      for (std::uint8_t byte : channel_out)
        if (auto parsed_frame = reader.push(byte); parsed_frame && parsed_frame->header.cls == FrameClass::State)
          received = deserialize(reader.payload());
    }

    // фільтр: predict щокроку, update коли є кадр
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
    if (received)
      std::printf("%.3f,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f,%.4f,%.4f\n",
                  time_s,
                  true_state.x,
                  true_state.y,
                  received->x,
                  received->y,
                  estimate.x,
                  estimate.y,
                  std::sqrt(estimate.x_var),
                  std::sqrt(estimate.y_var));
    else
      std::printf("%.3f,%.3f,%.3f,,,%.3f,%.3f,%.4f,%.4f\n",
                  time_s,
                  true_state.x,
                  true_state.y,
                  estimate.x,
                  estimate.y,
                  std::sqrt(estimate.x_var),
                  std::sqrt(estimate.y_var));
  }
  return 0;
}
