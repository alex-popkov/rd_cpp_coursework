#pragma once
#include <cstdint>
#include <optional>
#include "frame_reader.hpp"
#include "kalman2d.hpp"

namespace telemetry {

// Повний приймач STATE-телеметрії: байти -> кадри -> фільтр -> оцінка.
class StateReceiver {
public:
  StateReceiver(float sigma_a, float sigma_pos, float sigma_vel);

  // Згодовує байт. Повертає свіжу оцінку, коли STATE-кадр щойно оновив фільтр.
  std::optional<Estimate2D> push(std::uint8_t byte);

  Estimate2D estimate() const;

private:
  FrameReader reader_;
  Kalman2D filter_;
  bool initialized_ = false;
  std::uint32_t last_ts_ = 0;
};

}  // namespace telemetry