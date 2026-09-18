#include <gtest/gtest.h>
#include "trajectory.hpp"

using telemetry::circle_trajectory;

TEST(Trajectory, StartOfCircle)
{
  auto s = circle_trajectory(0.0f, 10.0f, 2.0f);  // R=10, omega=2
  EXPECT_NEAR(s.x, 10.0f, 1e-4);
  EXPECT_NEAR(s.y, 0.0f, 1e-4);
  EXPECT_NEAR(s.vx, 0.0f, 1e-4);
  EXPECT_NEAR(s.vy, 20.0f, 1e-4);  // R*omega
}

TEST(Trajectory, QuarterTurn)
{
  float R = 10.0f, w = 2.0f;
  float t = (3.14159265f / 2.0f) / w;  // omega*t = pi/2
  auto s = circle_trajectory(t, R, w);
  EXPECT_NEAR(s.x, 0.0f, 1e-3);
  EXPECT_NEAR(s.y, 10.0f, 1e-3);
  EXPECT_NEAR(s.vx, -20.0f, 1e-3);
  EXPECT_NEAR(s.vy, 0.0f, 1e-3);
}

TEST(Trajectory, VelocityMatchesDerivative)
{
  float R = 5.0f, w = 1.5f, t = 0.7f, e = 1e-3f;
  auto a = circle_trajectory(t - e, R, w);
  auto b = circle_trajectory(t + e, R, w);
  auto s = circle_trajectory(t, R, w);
  EXPECT_NEAR(s.vx, (b.x - a.x) / (2 * e), 1e-2);  // числова похідна збігається з vx
  EXPECT_NEAR(s.vy, (b.y - a.y) / (2 * e), 1e-2);
}