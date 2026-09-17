#include <gtest/gtest.h>
#include "kalman2d.hpp"

using telemetry::Kalman2D;

TEST(Kalman2D, UpdatePullsBothAxes)
{
  Kalman2D f(1.0f, 0.1f, 0.1f);
  f.update(3.0f, -4.0f, 1.0f, 2.0f);
  auto e = f.estimate();
  EXPECT_NEAR(e.x, 3.0f, 0.05f);
  EXPECT_NEAR(e.y, -4.0f, 0.05f);
  EXPECT_NEAR(e.vx, 1.0f, 0.05f);
  EXPECT_NEAR(e.vy, 2.0f, 0.05f);
}

TEST(Kalman2D, PredictAdvancesBothAxes)
{
  Kalman2D f(1.0f, 0.1f, 0.1f);
  f.update(0.0f, 0.0f, 1.0f, -2.0f);
  f.predict(2.0f);
  auto e = f.estimate();
  EXPECT_NEAR(e.x, 2.0f, 0.1f);   // 0 + 1·2
  EXPECT_NEAR(e.y, -4.0f, 0.1f);  // 0 + (−2)·2
}