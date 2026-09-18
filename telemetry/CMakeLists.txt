# Бібліотека-ядро: platform-agnostic C++, без системних викликів.
add_library(telemetry_core
    src/cobs.cpp
    src/crc16.cpp
    src/state_payload.cpp
    src/frame.cpp
    src/frame_reader.cpp
    src/seq_gap.cpp
    src/channel_sim.cpp
    src/kalman1d.cpp
    src/kalman2d.cpp
    src/state_receiver.cpp
    src/mock_hal.cpp
    src/posix_hal.cpp
    src/trajectory.cpp
)

# PUBLIC include: і сама бібліотека, і всі, хто до неї лінкується,
# бачать каталог include/ у своїх include-path.
target_include_directories(telemetry_core PUBLIC include)

# Ціль тестів.
add_executable(telemetry_tests
    tests/cobs_test.cpp
    tests/crc16_test.cpp
    tests/state_payload_test.cpp
    tests/frame_test.cpp
    tests/frame_reader_test.cpp
    tests/seq_gap_test.cpp
    tests/channel_sim_test.cpp
    tests/integration_test.cpp
    tests/kalman1d_test.cpp
    tests/kalman2d_test.cpp
    tests/state_receiver_test.cpp
    tests/mock_hal_test.cpp
    tests/trajectory_test.cpp
)

# Лінкуємо ядро + gtest_main (він дає готову функцію main()).
target_link_libraries(telemetry_tests
    PRIVATE
        telemetry_core
        GTest::gtest_main
)

add_executable(experiment apps/experiment.cpp)
target_link_libraries(experiment PRIVATE telemetry_core)

add_executable(rms apps/rms.cpp)
target_link_libraries(rms PRIVATE telemetry_core)

# Реєструє кожен TEST() у CTest окремо, щоб `ctest` бачив їх поіменно.
include(GoogleTest)
gtest_discover_tests(telemetry_tests)