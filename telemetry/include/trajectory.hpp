#pragma once

namespace telemetry {

struct TrueState {
  float x;
  float y;
  float vx;
  float vy;
};

// Рівномірний рух по колу: радіус radius, кутова швидкість omega (рад/с), час t (с).
TrueState circle_trajectory(float t, float radius, float omega);

}  // namespace telemetry