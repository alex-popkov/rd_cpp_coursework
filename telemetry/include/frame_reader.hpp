#pragma once
#include <array>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>

#include "frame.hpp"

namespace telemetry {

// Збирає кадри з байтового потоку: накопичує байти до роздільника 0x00, тоді розбирає.
class FrameReader {
public:
  FrameReader();
  // Згодовує один байт. Повертає ParsedFrame, якщо ЩОЙНО завершився валідний кадр;
  // nullopt інакше (кадр не завершено / порожній блок / пошкоджений — тихо відкинуто).
  // Байти payload завершеного кадру доступні через payload() до наступного push().
  std::optional<ParsedFrame> push(std::uint8_t byte);

  std::span<const std::uint8_t> payload() const { return {payload_.data(), payload_len_}; }

private:
  std::array<std::uint8_t, kMaxWireBlockSize> acc_{};  // накопичувач дротових байтів
  std::size_t acc_len_ = 0;
  bool overflow_ = false;  // кадр довший за максимум -> чекаємо роздільник

  std::array<std::uint8_t, kMaxPayloadSize> payload_{};  // сюди frame_decode кладе payload
  std::size_t payload_len_ = 0;
};

}  // namespace telemetry