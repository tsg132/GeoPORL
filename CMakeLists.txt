# Python bindings CMakeLists.txt

cmake_minimum_required(VERSION 3.10)
project(GeoPORL CXX)

set(CMAKE_CXX_STANDARD 17)

include_directories(include)

add_executable(run_example
    src/run_example.cpp
    src/env.cpp
    src/replay.cpp
    src/metrics.cpp
    src/dynamics.cpp
    src/shock.cpp
)