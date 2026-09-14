#include <gtest/gtest.h>
#include "frame.hpp"
#include "frame_reader.hpp"

#include <cstdint>
#include <utility>
#include <vector>

using telemetry::FrameClass;
using telemetry::FrameHeader;
using telemetry::FrameReader;

static std::vector<std::uint8_t> make_wire(const FrameHeader& hdr, const std::vector<std::uint8_t>& payload)
{
  std::vector<std::uint8_t> wire(telemetry::frame_encoded_max_size(payload.size()));
  std::size_t n = telemetry::frame_encode(hdr, payload, wire);
  wire.resize(n);
  return wire;
}

// Згодовує всі байти, збирає завершені кадри як (header, payload).
static std::vector<std::pair<FrameHeader, std::vector<std::uint8_t>>> feed(FrameReader& r, const std::vector<std::uint8_t>& bytes)
{
  std::vector<std::pair<FrameHeader, std::vector<std::uint8_t>>> out;
  for (std::uint8_t b : bytes) {
    if (auto f = r.push(b)) {
      auto p = r.payload();
      out.push_back({f->header, {p.begin(), p.end()}});
    }
  }
  return out;
}

TEST(FrameReader, SingleFrame)
{
  FrameReader r;
  auto frames = feed(r, make_wire({FrameClass::State, 5, 0, 0}, {0xAA, 0xBB}));
  ASSERT_EQ(frames.size(), 1u);
  EXPECT_EQ(frames[0].first.seq, 5);
  EXPECT_EQ(frames[0].second, (std::vector<std::uint8_t>{0xAA, 0xBB}));
}

TEST(FrameReader, TwoFramesBackToBack)
{
  FrameReader r;
  auto s = make_wire({FrameClass::State, 1, 0, 0}, {0x01});
  auto w2 = make_wire({FrameClass::Command, 2, 0, 0}, {0x02, 0x03});
  s.insert(s.end(), w2.begin(), w2.end());
  auto frames = feed(r, s);
  ASSERT_EQ(frames.size(), 2u);
  EXPECT_EQ(frames[0].first.seq, 1);
  EXPECT_EQ(frames[1].first.seq, 2);
  EXPECT_EQ(frames[1].second, (std::vector<std::uint8_t>{0x02, 0x03}));
}

TEST(FrameReader, IgnoresEmptyBlocksAndLeadingDelimiter)
{
  FrameReader r;
  std::vector<std::uint8_t> s = {0x00, 0x00};  // сміття-роздільники спереду
  auto wire = make_wire({FrameClass::State, 9, 0, 0}, {0x42});
  s.insert(s.end(), wire.begin(), wire.end());
  auto frames = feed(r, s);
  ASSERT_EQ(frames.size(), 1u);
  EXPECT_EQ(frames[0].first.seq, 9);
}

TEST(FrameReader, RecoversAfterCorruptFrame)
{
  FrameReader r;
  auto bad = make_wire({FrameClass::State, 1, 0, 0}, {0xDE, 0xAD});
  bad[bad.size() / 2] ^= 0xFF;  // псуємо середину, роздільник у кінці лишаємо
  auto good = make_wire({FrameClass::State, 2, 0, 0}, {0xBE, 0xEF});
  bad.insert(bad.end(), good.begin(), good.end());
  auto frames = feed(r, bad);
  ASSERT_EQ(frames.size(), 1u);  // битий відкинуто, добрий пройшов
  EXPECT_EQ(frames[0].first.seq, 2);
  EXPECT_EQ(frames[0].second, (std::vector<std::uint8_t>{0xBE, 0xEF}));
}

TEST(FrameReader, DiscardsOversizedFrameAndRecovers)
{
  FrameReader r;
  std::vector<std::uint8_t> stream(telemetry::kMaxWireBlockSize + 50, 0x41);  // задовгий «кадр»
  stream.push_back(0x00);                                                     // роздільник
  auto good = make_wire({FrameClass::State, 7, 0, 0}, {0x55});
  stream.insert(stream.end(), good.begin(), good.end());
  auto frames = feed(r, stream);
  ASSERT_EQ(frames.size(), 1u);  // задовгий відкинуто, добрий пройшов
  EXPECT_EQ(frames[0].first.seq, 7);
}