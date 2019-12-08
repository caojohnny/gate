cmake_minimum_required(VERSION 3.13)
project(gate VERSION 1.0 LANGUAGES C)

include(CMakePackageConfigHelpers)

set(MODULE_DIR "${CMAKE_CURRENT_LIST_DIR}")
get_filename_component(PARENT_DIR "${MODULE_DIR}" DIRECTORY)

include("${MODULE_DIR}/Doxygen.cmake")
include("${PARENT_DIR}/cmake/DownloadCspice.cmake")

add_library(gate
        gate/topo.h gate/topo.c
        gate/stars.c gate/stars.h
        gate/timeconv.c gate/timeconv.h
        gate/constants.h)
target_include_directories(gate
        PUBLIC "$<BUILD_INTERFACE:${MODULE_DIR}>"
        PUBLIC "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>")
target_link_libraries(gate
        PUBLIC cspice
        PRIVATE m)

include("${PARENT_DIR}/cmake/ExportLibrary.cmake")
