#include "cobs.hpp"
// Алгоритм кодування (псевдо, ~15 рядків реального C++):

// Тримай позицію code_pos у виході, куди пізніше впишеш лічильник; почни лічильник code = 1.
// Іди по вхідних байтах:
// байт ≠ 0 → скопіюй його у вихід, code++;
// байт = 0 → запиши code у code_pos, почни новий блок (новий code_pos, code = 1);
// якщо code досяг 0xFF (254 ненульових поспіль) → запиши 0xFF, почни новий блок без споживання нуля.
// Наприкінці запиши останній code у code_pos.
// Декодування — дзеркальне: читаєш code = n, копіюєш n-1 байтів як є, і якщо n != 0xFF та це не кінець — дописуєш один 0x00. Повторюєш.

namespace telemetry {

std::size_t cobs_encode(std::span<const std::uint8_t> in, std::span<std::uint8_t> out)
{
  std::uint8_t code_when_zero = 1;
  std::size_t code_pos = 0;
  std::size_t out_i = 1;

  for (std::size_t i = 0; i < in.size(); ++i) {
    if (in[i] == 0x00) {
      out[code_pos] = code_when_zero;
      code_pos = out_i;
      out_i++;
      code_when_zero = 1;
    }
    else {
      out[out_i] = in[i];
      out_i++;
      code_when_zero++;
    }

    if (code_when_zero == 0xFF) {
      out[code_pos] = code_when_zero;
      code_pos = out_i;
      out_i++;
      code_when_zero = 1;
    }
  }
  out[code_pos] = code_when_zero;

  return out_i;
}

std::optional<std::size_t> cobs_decode(std::span<const std::uint8_t> in, std::span<std::uint8_t> out)
{
  return 0;
}

}  // namespace telemetry