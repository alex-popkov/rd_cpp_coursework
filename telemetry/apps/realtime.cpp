#include "trajectory.hpp"
#include "state_payload.hpp"
#include "frame.hpp"
#include "state_receiver.hpp"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <deque>
#include <random>
#include <vector>

using namespace telemetry;

// --- Шар 2: Gilbert–Elliott. Втрати кластеряться (стани good/bad),
// а не рівномірні iid, як у ChannelSim. Блекаут виникає сам — як довге
// перебування в bad, без окремого if. ---
class BurstChannel {
public:
  BurstChannel(double p_g2b, double p_b2g, double loss_good, double loss_bad, std::uint32_t seed)
    : p_g2b_(p_g2b)
    , p_b2g_(p_b2g)
    , lg_(loss_good)
    , lb_(loss_bad)
    , rng_(seed)
  {
  }

  bool drop()  // true -> кадр втрачено на цьому кроці
  {
    std::uniform_real_distribution<double> u(0.0, 1.0);
    if (bad_) {
      if (u(rng_) < p_b2g_)
        bad_ = false;
    }
    else {
      if (u(rng_) < p_g2b_)
        bad_ = true;
    }
    return u(rng_) < (bad_ ? lb_ : lg_);
  }
  bool bad() const { return bad_; }

private:
  double p_g2b_, p_b2g_, lg_, lb_;
  bool bad_ = false;
  std::mt19937 rng_;
};

int main()
{
  const int step_count = 400;
  const float dt = 0.05f;  // 20 Гц — бортовий семпл
  const float radius = 10.0f;
  const float omega = 0.5f;
  const float noise_pos = 0.20f;
  const float noise_vel = 0.10f;

  StateReceiver rx(/*sigma_a=*/1.0f, noise_pos, noise_vel);

  // p_g2b малий (рідко зриваємось), p_b2g помірний (завмирання триває кілька
  // кадрів), у bad майже все гине -> природні кластери втрат і «блекаути».
  BurstChannel channel(/*p_g2b=*/0.02, /*p_b2g=*/0.15, /*loss_good=*/0.02, /*loss_bad=*/0.95, /*seed=*/42);

  std::mt19937 noise_rng(7);
  std::normal_distribution<float> pos_noise(0.0f, noise_pos);
  std::normal_distribution<float> vel_noise(0.0f, noise_vel);

  // --- Шар 1: рваний read. Радіо віддає байти клубками; кадр рветься й
  // склеюється між викликами read. chunk у середньому > розміру кадру (~30 б),
  // тож черга не росте безмежно, але часто менший за кадр -> кадр ділиться. ---
  std::mt19937 read_rng(99);
  std::uniform_int_distribution<int> chunk(5, 75);

  std::deque<std::uint8_t> pipe;  // «ефір»: доставлені байти чекають на read
  std::vector<std::uint8_t> payload(kStatePayloadSize);

  std::printf("t,true_x,true_y,est_x,est_y,sig_x,sig_y,delivered,link_bad\n");

  bool seeded = false;

  for (int step = 0; step < step_count; ++step) {
    float t = step * dt;
    TrueState truth = circle_trajectory(t, radius, omega);

    // передавач: кадр STATE з бортовою міткою часу (вона веде фільтр).
    float mx = truth.x + pos_noise(noise_rng);
    float my = truth.y + pos_noise(noise_rng);
    float mvx = truth.vx + vel_noise(noise_rng);
    float mvy = truth.vy + vel_noise(noise_rng);
    StatePayload sp{static_cast<std::uint32_t>(t * 1000.0f), mx, my, mvx, mvy};
    serialize(sp, payload);

    std::vector<std::uint8_t> wire(frame_encoded_max_size(payload.size()));
    std::size_t wlen = frame_encode({FrameClass::State, static_cast<std::uint8_t>(step), 0, 0}, payload, wire);
    wire.resize(wlen);

    // Шар 2: канал вирішує долю кадру (крок 0 форсуємо, щоб фільтр засіявся).
    bool dropped = (step != 0) && channel.drop();
    if (!dropped)
      for (std::uint8_t b : wire)
        pipe.push_back(b);  // байти кадру -> в ефір

    // Шар 1: рваний read -> самотактований push() (справжній код-шлях приймача).
    bool delivered = false;
    int take = std::min<int>(chunk(read_rng), static_cast<int>(pipe.size()));
    for (int i = 0; i < take; ++i) {
      if (rx.push(pipe.front())) {  // кадр щойно оновив фільтр
        delivered = true;
        seeded = true;
      }
      pipe.pop_front();
    }

    if (!seeded)
      continue;  // ще чекаємо перший засівний кадр

    Estimate2D e = rx.estimate();  // самотактований: між кадрами заморожено
    std::printf("%.3f,%.3f,%.3f,%.3f,%.3f,%.4f,%.4f,%d,%d\n",
                t,
                truth.x,
                truth.y,
                e.x,
                e.y,
                std::sqrt(e.x_var),
                std::sqrt(e.y_var),
                delivered ? 1 : 0,
                channel.bad() ? 1 : 0);
  }
  return 0;
}