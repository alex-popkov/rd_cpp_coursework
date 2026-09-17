#pragma once
#include "mat2.hpp"

namespace telemetry {

struct Estimate1D {
  float pos;
  float vel;
  float pos_var;  // P.m00 — дисперсія позиції, для коридору ±3σ
};

// 1D фільтр Калмана, модель постійної швидкості. Стан [pos, vel], вимірюємо обидва (H = I).
class Kalman1D {
public:
  // sigma_a — середнєквад відхилення шуму прискорення (Q); sigma_pos/sigma_vel — СКВ шуму вимірювання (R).
  Kalman1D(float sigma_a, float sigma_pos, float sigma_vel);

  void predict(float dt);
  void update(float pos_meas, float vel_meas);
  Estimate1D estimate() const;

private:
  Vec2 x_{}; // стан [pos, vel]
  Mat2 P_{1e6f, 0.0f, 0.0f, 1e6f};  // коваріація, старт «нічого не знаємо»
  float sa2_; // sigma_a^2
  Mat2 R_; // шум вимірювання (діагональна)
};

}  // namespace telemetry