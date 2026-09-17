#include "kalman2d.hpp"

namespace telemetry {

Kalman2D::Kalman2D(float sigma_a, float sigma_pos, float sigma_vel)
  : fx_(sigma_a, sigma_pos, sigma_vel)
  , fy_(sigma_a, sigma_pos, sigma_vel)
{
}

void Kalman2D::predict(float dt)
{
  fx_.predict(dt);
  fy_.predict(dt);
}

void Kalman2D::update(float x, float y, float vx, float vy)
{
  fx_.update(x, vx);
  fy_.update(y, vy);
}

Estimate2D Kalman2D::estimate() const
{
  Estimate1D ex = fx_.estimate();
  Estimate1D ey = fy_.estimate();

  return {ex.pos, ey.pos, ex.vel, ey.vel, ex.pos_var, ey.pos_var};
}

}  // namespace telemetry