#include "kalman1d.hpp"

namespace telemetry {

Kalman1D::Kalman1D(float sigma_a, float sigma_pos, float sigma_vel)
  : sa2_(sigma_a * sigma_a)
  , R_{sigma_pos * sigma_pos, 0.0f, 0.0f, sigma_vel * sigma_vel}
{
}

void Kalman1D::predict(float dt)
{
  Mat2 F{1.0f, dt, 0.0f, 1.0f};

  float dt2 = dt * dt;
  float dt3 = dt2 * dt;
  float dt4 = dt3 * dt;

  Mat2 Q = sa2_ * Mat2{dt4 / 4.0f, dt3 / 2.0f, dt3 / 2.0f, dt2};

  x_ = F * x_;
  P_ = F * P_ * transpose(F) + Q;
}

void Kalman1D::update(float pos_meas, float vel_meas)
{
  Vec2 z{pos_meas, vel_meas};
  Vec2 y = z - x_; // інновація
  Mat2 S = P_ + R_; // бо H = I
  Mat2 K = P_ * inverse(S); // gain
  x_ = x_ + K * y; // корекція
  P_ = (Mat2::identity() - K) * P_; // (I − K)·P
}

Estimate1D Kalman1D::estimate() const
{
  return {x_.a, x_.b, P_.m00};
}

}  // namespace telemetry