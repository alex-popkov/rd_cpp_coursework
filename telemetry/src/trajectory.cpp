#include "trajectory.hpp"
#include <cmath>

namespace telemetry {

TrueState circle_trajectory(float t, float radius, float omega)
{
  float a = omega * t;
  float x = radius * std::cos(a);
  float y = radius * std::sin(a);
  float vx = -radius * omega * std::sin(a);
  float vy = radius * omega * std::cos(a);

  return {x, y, vx, vy};
};

}  // namespace telemetry