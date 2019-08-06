cmake_minimum_required(VERSION 3.13)
project(libgate VERSION 1.0 LANGUAGES C)

include(../cmake/DownloadCspice.cmake)

add_library(gate topo.h topo.c stars.c stars.h constants.h)
target_include_directories(gate PUBLIC .)
target_link_libraries(gate
        PUBLIC cspice
        PUBLIC m)
