#include "main.h"
#include "trajectory.hpp"
#include "state_payload.hpp"
#include "frame.hpp"

#include <array>
#include <cstdint>

extern UART_HandleTypeDef huart2;

using namespace telemetry;

extern "C" void app_loop(void) {
    static std::uint8_t seq = 0;

    std::uint32_t now_ms = HAL_GetTick();          // бортова мітка часу
    float t = now_ms / 1000.0f;
    TrueState s = circle_trajectory(t, 10.0f, 0.5f);

    StatePayload payload_struct{now_ms, s.x, s.y, s.vx, s.vy};

    std::array<std::uint8_t, kStatePayloadSize> payload{};
    serialize(payload_struct, payload);

    std::array<std::uint8_t, frame_encoded_max_size(kStatePayloadSize)> wire{};
    std::size_t n = frame_encode({FrameClass::State, seq, 0, 0}, payload, wire);

    HAL_UART_Transmit(&huart2, wire.data(), (uint16_t)n, HAL_MAX_DELAY);

    HAL_GPIO_TogglePin(LD2_GPIO_Port, LD2_Pin);
    seq++;
    HAL_Delay(100);   // 10 Гц
}