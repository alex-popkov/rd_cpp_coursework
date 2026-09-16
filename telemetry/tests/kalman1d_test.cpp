#include <gtest/gtest.h>
#include "kalman1d.hpp"

using telemetry::Kalman1D;

TEST(Kalman1D, UpdatePullsTowardMeasurement)
{
  Kalman1D f(1.0f, 0.1f, 0.1f);
  f.update(5.0f, 1.0f);  // старт P величезний -> віримо виміру
  auto e = f.estimate();
  EXPECT_NEAR(e.pos, 5.0f, 0.05f);
  EXPECT_NEAR(e.vel, 1.0f, 0.05f);
}

TEST(Kalman1D, PredictAdvancesByVelocity)
{
  Kalman1D f(1.0f, 0.1f, 0.1f);
  f.update(0.0f, 2.0f);  // стан ~ [0, 2]
  f.predict(1.0f);       // pos += vel*dt
  auto e = f.estimate();
  EXPECT_NEAR(e.pos, 2.0f, 0.1f);
  EXPECT_NEAR(e.vel, 2.0f, 0.1f);
}

TEST(Kalman1D, PredictGrowsUncertaintyUpdateShrinks)
{
  Kalman1D f(1.0f, 0.1f, 0.1f);
  f.update(0.0f, 0.0f);
  float after_update = f.estimate().pos_var;
  f.predict(1.0f);
  float after_predict = f.estimate().pos_var;
  EXPECT_GT(after_predict, after_update);  // передбачення роздуває
  f.update(0.0f, 0.0f);
  EXPECT_LT(f.estimate().pos_var, after_predict);  // вимір стискає
}

TEST(Kalman1D, TracksConstantVelocity)
{
  Kalman1D f(0.01f, 0.1f, 0.1f);
  const float v = 2.0f, dt = 1.0f;
  for (int k = 1; k <= 10; ++k) {
    f.predict(dt);
    f.update(v * k, v);  // точні виміри траєкторії pos = v*k
  }
  auto e = f.estimate();
  EXPECT_NEAR(e.pos, 20.0f, 0.5f);
  EXPECT_NEAR(e.vel, 2.0f, 0.2f);
}