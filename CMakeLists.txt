# cmake_minimum_required(VERSION <specify CMake version here>)
project(daylight)

set(CMAKE_CXX_STANDARD 14)

include_directories(include)

add_executable(daylight
        include/CLI11.hpp
        src/main.cpp)
