#pragma once
#include <cstddef>
#include <cstdint>
#include <optional>
#include <random>
#include <span>

namespace telemetry {

// Модель радіоканалу: втрата кадру цілком + опційна інверсія одного біта.
// Детермінований за сідом -> відтворювані тести.
class ChannelSim {
public:
  ChannelSim(double loss_prob, double bitflip_prob, std::uint32_t seed);

  // Пропускає кадр через канал.
  //   nullopt        -> кадр втрачено;
  //   *value == len   -> кадр доставлено; байти в out (можливо, з переверненим бітом).
  // out мусить вміщати щонайменше frame.size() байтів.
  std::optional<std::size_t> transmit(std::span<const std::uint8_t> frame, std::span<std::uint8_t> out);

private:
  double loss_prob_;
  double bitflip_prob_;
  std::mt19937 rng_;
};

}  // namespace telemetry