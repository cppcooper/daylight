cmake_minimum_required(VERSION 3.19)
project(daylight)

set(CMAKE_CXX_STANDARD 20)

include_directories(include)

add_executable(daylight
        include/CLI11.hpp
        src/main.cpp src/time.cpp src/solar.cpp)

install(TARGETS daylight DESTINATION bin)