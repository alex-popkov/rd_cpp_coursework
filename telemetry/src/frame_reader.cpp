#include "frame_reader.hpp"
#include "frame.hpp"

namespace telemetry {

FrameReader::FrameReader() {}

std::optional<ParsedFrame> FrameReader::push(std::uint8_t byte)
{
  if (byte == 0x00) {
    // end of the frame
    if (this->overflow_ || this->acc_len_ == 0) {
      this->overflow_ = false;
      this->acc_len_ = 0;

      return std::nullopt;
    }

    auto parsed_frame = frame_decode({this->acc_.data(), this->acc_len_}, this->payload_);
    if (parsed_frame) {
      this->payload_len_ = parsed_frame->payload_len;
    }

    this->acc_len_ = 0;

    return parsed_frame;
  }

  if (this->overflow_) {
    return std::nullopt;
  }

  if (this->acc_len_ >= kMaxWireBlockSize) {
    this->overflow_ = true;

    return std::nullopt;
  }
  this->acc_[this->acc_len_] = byte;
  this->acc_len_++;

  return std::nullopt;
}

}  // namespace telemetry