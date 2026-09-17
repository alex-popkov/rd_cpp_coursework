#include <gtest/gtest.h>
#include "state_receiver.hpp"
#include "frame.hpp"
#include "state_payload.hpp"

#include <cstdint>
#include <optional>
#include <vector>

using namespace telemetry;

static std::vector<std::uint8_t> make_state_wire(std::uint8_t seq, const StatePayload& p)
{
  std::vector<std::uint8_t> payload(kStatePayloadSize);
  serialize(p, payload);
  std::vector<std::uint8_t> wire(frame_encoded_max_size(payload.size()));
  std::size_t n = frame_encode({FrameClass::State, seq, 0, 0}, payload, wire);
  wire.resize(n);
  return wire;
}

TEST(StateReceiver, FirstFrameSeeds)
{
  StateReceiver rx(1.0f, 0.1f, 0.1f);
  std::optional<Estimate2D> last;
  for (auto b : make_state_wire(0, {1000, 5.0f, 3.0f, 1.0f, 2.0f}))
    last = rx.push(b);
  ASSERT_TRUE(last.has_value());
  EXPECT_NEAR(last->x, 5.0f, 0.1f);
  EXPECT_NEAR(last->y, 3.0f, 0.1f);
}

TEST(StateReceiver, PredictsAcrossDroppedFrame)
{
  StateReceiver rx(0.01f, 0.1f, 0.1f);
  for (auto b : make_state_wire(0, {0, 0.0f, 0.0f, 10.0f, 0.0f}))
    rx.push(b);  // seed t=0
  // кадр t=1000 "загублено" (не подаємо); наступний прийнятий — t=2000, dt=2
  std::optional<Estimate2D> last;
  for (auto b : make_state_wire(2, {2000, 20.0f, 0.0f, 10.0f, 0.0f}))
    last = rx.push(b);
  ASSERT_TRUE(last.has_value());
  EXPECT_NEAR(last->x, 20.0f, 0.5f);  // великий dt сам врахував прогалину
  EXPECT_NEAR(last->vx, 10.0f, 0.5f);
}

TEST(StateReceiver, IgnoresStaleFrame)
{
  StateReceiver rx(1.0f, 0.1f, 0.1f);
  for (auto b : make_state_wire(0, {2000, 5.0f, 0.0f, 0.0f, 0.0f}))
    rx.push(b);
  std::optional<Estimate2D> last;
  for (auto b : make_state_wire(1, {1000, 99.0f, 0.0f, 0.0f, 0.0f}))
    last = rx.push(b);
  EXPECT_FALSE(last.has_value());            // застаріле відкинуто
  EXPECT_NEAR(rx.estimate().x, 5.0f, 0.1f);  // оцінка не зіпсована
}