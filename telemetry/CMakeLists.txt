# Бібліотека-ядро: platform-agnostic C++, без системних викликів.
add_library(telemetry_core
    src/cobs.cpp
    src/crc16.cpp
)

# PUBLIC include: і сама бібліотека, і всі, хто до неї лінкується,
# бачать каталог include/ у своїх include-path.
target_include_directories(telemetry_core PUBLIC include)

# Ціль тестів.
add_executable(telemetry_tests
    tests/cobs_test.cpp
    tests/crc16_test.cpp
)

# Лінкуємо ядро + gtest_main (він дає готову функцію main()).
target_link_libraries(telemetry_tests
    PRIVATE
        telemetry_core
        GTest::gtest_main
)

# Реєструє кожен TEST() у CTest окремо, щоб `ctest` бачив їх поіменно.
include(GoogleTest)
gtest_discover_tests(telemetry_tests)