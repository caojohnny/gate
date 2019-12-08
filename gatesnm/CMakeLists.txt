cmake_minimum_required(VERSION 3.13)
project(gatesnm VERSION 1.0 LANGUAGES C)

set(MODULE_DIR "${CMAKE_CURRENT_LIST_DIR}")
get_filename_component(PARENT_DIR "${MODULE_DIR}" DIRECTORY)

add_library(gatesnm gatesnm/snm.c gatesnm/snm.h)
target_include_directories(gatesnm
        PUBLIC "$<BUILD_INTERFACE:${MODULE_DIR}>"
        PUBLIC "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>")
target_link_libraries(gatesnm
        PRIVATE gate)

include("${PARENT_DIR}/cmake/ExportLibrary.cmake")

