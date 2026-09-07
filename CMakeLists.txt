cmake_minimum_required(VERSION 3.20)

# Загальний CMake-проєкт для курсової роботи.
# Кожна директорія підключається окремо.
project(rd_cpp_coursework LANGUAGES CXX)

# C++20 - базовий стандарт курсу. EXTENSIONS OFF прибирає GNU-розширення,
# щоб код лишався ближчим до переносимого ISO C++.
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# clangd читає compile_commands.json і завдяки цьому бачить ті самі include
# paths, defines і flags, що й реальна CMake-збірка.
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# GoogleTest підтягуємо один раз. 
include(FetchContent)
FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
FetchContent_MakeAvailable(googletest)

# Вмикає CTest.
enable_testing()

