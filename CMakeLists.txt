cmake_minimum_required(VERSION 3.20)

project(main
   VERSION 0.1.0
   LANGUAGES CXX)


set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(main
   src/main.cpp
   src/bitboard.cpp
   src/position.cpp
   src/test.cpp
   src/bitset.cpp
   src/matcher.cpp)

   target_include_directories(main PRIVATE ${CMAKE_SOURCE_DIR}/external/csv2/csv2/include)

target_compile_options(main PRIVATE /arch:AVX2)