#include <gtest/gtest.h>
#include "seq_gap.hpp"

using telemetry::SeqGapCounter;

TEST(SeqGap, FirstFrameHasNoGap)
{
  SeqGapCounter c;
  EXPECT_EQ(c.observe(5), 0);
}

TEST(SeqGap, ConsecutiveNoGap)
{
  SeqGapCounter c;
  c.observe(5);
  EXPECT_EQ(c.observe(6), 0);
}

TEST(SeqGap, OneMissed)
{
  SeqGapCounter c;
  c.observe(5);
  EXPECT_EQ(c.observe(7), 1);  // пропущено 6
}

TEST(SeqGap, SeveralMissed)
{
  SeqGapCounter c;
  c.observe(10);
  EXPECT_EQ(c.observe(20), 9);  // 11..19
}

TEST(SeqGap, WrapAround)
{
  SeqGapCounter c;
  c.observe(254);
  EXPECT_EQ(c.observe(1), 2);  // пропущені 255 і 0
}