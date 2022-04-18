cmake_minimum_required(VERSION 3.19)
project(team32 CXX C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_CXX_STANDARD 14)

add_compile_options(-Wall -g -fsanitize=address -fsanitize=undefined)
add_link_options(-fsanitize=address -g -fsanitize=undefined)

add_subdirectory(src)