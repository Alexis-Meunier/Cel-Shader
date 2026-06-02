cmake_minimum_required(VERSION 3.21)
project(CelShading)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
include_directories(.)

add_executable(CelShading
    matrix.cc  program.cc point.cc object.cc texture.cc main.cpp
)

add_compile_options(CelShading PRIVATE -Wall -Wextra -O3 -g -std=c++20 -m64 -march=native -fopt-info-vec-optimized)
target_link_libraries(CelShading PRIVATE -lGL -lGLEW -lglut -lpthread)