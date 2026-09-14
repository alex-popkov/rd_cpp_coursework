#include <array>
#include "frame.hpp"
#include "crc16.hpp"

namespace telemetry {

// frame
//  [LEN][CLASS][SEQ][ACK][BITMAP][ payload×LEN ][ CRC:2 ]   ← далі все це через COBS + 0x00
//   └──────── CRC16 над цим діапазоном ────────┘

std::size_t frame_encode(const FrameHeader& header, std::span<const std::uint8_t> payload, std::span<std::uint8_t> out)
{
  constexpr std::size_t kMaxContentSize = kFrameHeaderSize + kMaxPayloadSize + kFrameCrcSize;  // 262
  std::array<std::uint8_t, kMaxContentSize> content;

  if (payload.size() > kMaxPayloadSize) {
    return 0;  // LEN — один байт, більший payload непредставний
  }

  content[0] = payload.size();  // len
  content[1] = static_cast<uint8_t>(header.cls);
  content[2] = static_cast<uint8_t>(header.seq);
  content[3] = static_cast<uint8_t>(header.ack);
  content[4] = static_cast<uint8_t>(header.bitmap);
  for (std::size_t i = 0; i < payload.size(); ++i) {
    content[kFrameHeaderSize + i] = payload[i];
  }

  std::size_t content_len = kFrameHeaderSize + content[0];
  std::uint16_t crc = crc16_ccitt({content.data(), content_len});
  content[kFrameHeaderSize + content[0]] = crc & 0xFF;             // пишемо нижній байт
  content[kFrameHeaderSize + content[0] + 1] = (crc >> 8) & 0xFF;  // пишемо верхній байт
  content_len += kFrameCrcSize;                                    // додалось 2 байти crc
  std::size_t n = cobs_encode({content.data(), content_len}, out);
  out[n] = 0x00;
  n++;

  return n;
}

std::optional<ParsedFrame> frame_decode(std::span<const std::uint8_t> block, std::span<std::uint8_t> payload_out)
{
  constexpr std::size_t kMaxContentSize = kFrameHeaderSize + kMaxPayloadSize + kFrameCrcSize;  // 262
  std::array<std::uint8_t, kMaxContentSize> content;
  auto decoded = cobs_decode(block, content);
  if (!decoded) {
    return std::nullopt;
  }

  std::size_t real_len = *decoded;
  if (real_len < kFrameHeaderSize + kFrameCrcSize) {
    return std::nullopt;
  }

  std::size_t content_len = kFrameHeaderSize + content[0];
  if (content_len + kFrameCrcSize != real_len) {
    return std::nullopt;
  }

  std::uint16_t crc_calculated = crc16_ccitt({content.data(), content_len});
  std::uint16_t crc_wire = content[kFrameHeaderSize + content[0]] | (content[kFrameHeaderSize + content[0] + 1] << 8);

  if (crc_calculated != crc_wire) {
    return std::nullopt;
  }

  FrameHeader header{static_cast<FrameClass>(content[1]), content[2], content[3], content[4]};

  for (std::size_t i = 0; i < content[0]; ++i) {
    payload_out[i] = content[i + kFrameHeaderSize];
  }

  return ParsedFrame{header, content[0]};
};

}  // namespace telemetry