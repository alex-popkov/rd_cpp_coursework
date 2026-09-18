#include "posix_hal.hpp"

#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#include <time.h>
#include <stdexcept>

namespace telemetry {

static speed_t to_speed(int baud) {
    switch (baud) {
        case 9600: return B9600;
        case 19200: return B19200;
        case 38400: return B38400;
        case 57600: return B57600;
        case 115200: return B115200;
        case 230400: return B230400;
        default: return B115200;
    }
}

PosixHal::PosixHal(const std::string& device, int baud) {
    fd_ = ::open(device.c_str(), O_RDWR | O_NOCTTY | O_NONBLOCK);
    if (fd_ < 0) throw std::runtime_error("PosixHal: cannot open " + device);

    termios tty{};
    if (::tcgetattr(fd_, &tty) != 0) { ::close(fd_); throw std::runtime_error("tcgetattr failed"); }

    cfmakeraw(&tty); // сирий режим: без обробки, байт-у-байт
    cfsetispeed(&tty, to_speed(baud));
    cfsetospeed(&tty, to_speed(baud));
    tty.c_cc[VMIN]  = 0; // read не блокує — віддає що є
    tty.c_cc[VTIME] = 0;

    if (::tcsetattr(fd_, TCSANOW, &tty) != 0) 
    { 
        ::close(fd_); 
        throw std::runtime_error("tcsetattr failed"); 
    }
}

PosixHal::~PosixHal() 
{ if (fd_ >= 0) ::close(fd_); }

std::size_t PosixHal::read_bytes(std::span<std::uint8_t> buf) {
    ssize_t n = ::read(fd_, buf.data(), buf.size());
    return (n > 0) ? static_cast<std::size_t>(n) : 0;   // 0 = нема даних (EAGAIN/порожньо)
}

std::size_t PosixHal::write_bytes(std::span<const std::uint8_t> data) {
    ssize_t n = ::write(fd_, data.data(), data.size());
    return (n > 0) ? static_cast<std::size_t>(n) : 0;
}

std::uint64_t PosixHal::now_us() const {
    timespec ts{};
    clock_gettime(CLOCK_MONOTONIC, &ts); // монотонний -> не стрибає при корекції системного часу
    return static_cast<std::uint64_t>(ts.tv_sec) * 1000000ULL + ts.tv_nsec / 1000ULL;
}

}  // namespace telemetry