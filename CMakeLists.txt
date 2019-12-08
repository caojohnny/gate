cmake_minimum_required(VERSION 3.13)
project(gate-parent VERSION 1.0 LANGUAGES C)

set(CMAKE_C_STANDARD 99)

add_subdirectory(gate)
add_subdirectory(gatesnm)
add_subdirectory(gatecli)
