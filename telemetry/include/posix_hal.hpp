#pragma once
#include <string>
#include "hal.hpp"

namespace telemetry {

// POSIX/termios реалізація HAL: реальний serial-порт або socat pty.
class PosixHal : public IHal {
public:
  explicit PosixHal(const std::string& device, int baud = 115200);  // кидає на помилці open
  ~PosixHal() override;

  std::size_t read_bytes(std::span<std::uint8_t> buf) override;
  std::size_t write_bytes(std::span<const std::uint8_t> data) override;
  std::uint64_t now_us() const override;

private:
  int fd_ = -1;
};

}  // namespace telemetry