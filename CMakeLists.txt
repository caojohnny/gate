cmake_minimum_required(VERSION 3.13)
project(gate VERSION 1.0 LANGUAGES C)

set(CMAKE_C_STANDARD 99)

add_subdirectory(src)
add_subdirectory(cli)
