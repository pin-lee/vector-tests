cmake_minimum_required(VERSION 3.26)
project(untitled)

set(CMAKE_CXX_STANDARD 17)

add_executable(untitled main.cpp)

if (GCC)
    add_compile_options( -O3) # https://stackoverflow.com/questions/10366670/how-to-compile-simd-code-with-gcc
endif()