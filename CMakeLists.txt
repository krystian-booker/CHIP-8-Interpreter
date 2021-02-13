cmake_minimum_required(VERSION 3.17)
project(chip8_interpreter)

set(CMAKE_CXX_STANDARD 14)

add_executable(chip8_interpreter main.cpp Core.cpp Core.h)