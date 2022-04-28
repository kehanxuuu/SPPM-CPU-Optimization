cmake_minimum_required(VERSION 3.19)
project(team32 CXX C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_CXX_STANDARD 14)

set(CMAKE_C_FLAGS "-O3 -ffast-math")

add_subdirectory(src)