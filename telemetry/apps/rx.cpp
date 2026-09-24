#include "posix_hal.hpp"
#include "state_receiver.hpp"

#include <array>
#include <cstdint>
#include <cstdio>
#include <string>
#include <unistd.h>

using namespace telemetry;

int main(int argc, char** argv)
{
  std::string device = (argc > 1) ? argv[1] : "/dev/cu.usbmodem11103";
  PosixHal hal(device, 115200);
  StateReceiver rx(/*sigma_a=*/1.0f, /*sigma_pos=*/0.2f, /*sigma_vel=*/0.1f);

  std::printf("est_x,est_y,est_vx,est_vy\n");
  std::array<std::uint8_t, 256> buf{};
  while (true) {
    std::size_t n = hal.read_bytes(buf);
    for (std::size_t i = 0; i < n; ++i) {
      if (auto e = rx.push(buf[i])) {
        std::printf("%.3f,%.3f,%.3f,%.3f\n", e->x, e->y, e->vx, e->vy);
        std::fflush(stdout);
      }
    }
    if (n == 0)
      usleep(1000);  // не крутити CPU даремно
  }
}