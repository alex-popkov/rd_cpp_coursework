#include <gtest/gtest.h>
#include "state_payload.hpp"
#include "frame.hpp"
#include "frame_reader.hpp"
#include "seq_gap.hpp"
#include "channel_sim.hpp"

#include <cstdint>
#include <vector>

using namespace telemetry;

struct RxStats {
  int delivered = 0;
  long total_missed = 0;  // сума прогалин за SEQ
  bool have_first = false;
  int first_seq = 0;
  int last_seq = 0;
};

// Жене n кадрів телеметрії через канал і збирає статистику на приймачі.
static RxStats run_pipeline(int n, double loss, double bitflip, std::uint32_t seed)
{
  ChannelSim channel(loss, bitflip, seed);
  FrameReader reader;
  SeqGapCounter gaps;
  RxStats st;

  std::vector<std::uint8_t> payload(kStatePayloadSize);
  for (int i = 0; i < n; ++i) {  // n <= 256 -> SEQ без wrap
    StatePayload p{static_cast<std::uint32_t>(i), 1.0f * i, 2.0f * i, 0.0f, 0.0f};
    serialize(p, payload);

    std::vector<std::uint8_t> wire(frame_encoded_max_size(payload.size()));
    std::size_t wlen = frame_encode({FrameClass::State, static_cast<std::uint8_t>(i), 0, 0}, payload, wire);
    wire.resize(wlen);

    std::vector<std::uint8_t> out(wire.size());
    auto delivered = channel.transmit(wire, out);  // TX -> канал
    if (!delivered)
      continue;  // кадр втрачено
    out.resize(*delivered);

    for (std::uint8_t byte : out) {  // RX: байти -> кадри
      if (auto frame = reader.push(byte)) {
        st.delivered++;
        std::uint8_t s = frame->header.seq;
        std::uint8_t g = gaps.observe(s);
        if (st.have_first)
          st.total_missed += g;
        else {
          st.have_first = true;
          st.first_seq = s;
        }
        st.last_seq = s;
      }
    }
  }
  return st;
}

TEST(Integration, NoLossDeliversAll)
{
  RxStats st = run_pipeline(200, 0.0, 0.0, 1);
  EXPECT_EQ(st.delivered, 200);
  EXPECT_EQ(st.total_missed, 0);
}

TEST(Integration, LossySeqAccountingConsistent)
{
  RxStats st = run_pipeline(200, 0.4, 0.0, 123);
  ASSERT_TRUE(st.have_first);
  EXPECT_LT(st.delivered, 200);  // частину загублено
  EXPECT_GT(st.delivered, 0);
  // ІНВАРІАНТА: доставлені + пропущені(за SEQ) == діапазон номерів
  EXPECT_EQ(st.delivered + st.total_missed, st.last_seq - st.first_seq + 1);
}

TEST(Integration, CorruptionCountsAsMissed)
{
  RxStats st = run_pipeline(200, 0.2, 0.3, 77);  // втрати + биті кадри
  ASSERT_TRUE(st.have_first);
  // битий кадр = не долетів -> та сама інваріанта тримається
  EXPECT_EQ(st.delivered + st.total_missed, st.last_seq - st.first_seq + 1);
}