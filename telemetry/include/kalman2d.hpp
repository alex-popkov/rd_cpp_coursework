#pragma once
#include "kalman1d.hpp"

namespace telemetry {

struct Estimate2D {
  float x, y;
  float vx, vy;
  float x_var, y_var;  // дисперсії позиції по осях -> коридор ±3σ
};

class Kalman2D {
public:
  Kalman2D(float sigma_a, float sigma_pos, float sigma_vel);
  void predict(float dt);
  void update(float x, float y, float vx, float vy);
  Estimate2D estimate() const;

private:
  Kalman1D fx_;
  Kalman1D fy_;
};

}  // namespace telemetry