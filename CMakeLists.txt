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
   src/matcher.cpp
   src/moves.cpp
   src/relation.cpp
   
   src/file_io.cpp
   )

target_compile_options(main PRIVATE /arch:AVX2)