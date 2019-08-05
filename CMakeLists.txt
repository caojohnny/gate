cmake_minimum_required(VERSION 3.13)
project(gate VERSION 1.0 LANGUAGES C)

set(CMAKE_C_STANDARD 99)

include(FetchContent)

FetchContent_Declare(
        cspice
        URL http://naif.jpl.nasa.gov/pub/naif/toolkit/C/PC_Linux_GCC_64bit/packages/cspice.tar.Z
        URL_HASH SHA256=93cd4fbce5818f8b7fecf3914c5756b8d41fd5bdaaeac1f4037b5a5410bc4768
        PATCH_COMMAND patch -r - -i ${CMAKE_SOURCE_DIR}/patch/Fix-compilation-issue-with-SpiceZpr.patch <SOURCE_DIR>/include/SpiceZpr.h || true
)
FetchContent_GetProperties(cspice)
if (NOT cspice_POPULATED)
    FetchContent_Populate(cspice)
endif ()

add_library(cspice STATIC IMPORTED)
set_property(TARGET cspice PROPERTY IMPORTED_LOCATION
        "${cspice_SOURCE_DIR}/lib/cspice.a")
target_include_directories(cspice INTERFACE ${cspice_SOURCE_DIR}/include)

add_library(csupport STATIC IMPORTED)
set_property(TARGET csupport PROPERTY IMPORTED_LOCATION
        "${cspice_SOURCE_DIR}/lib/csupport.a")

add_library(dependencies INTERFACE)
target_link_libraries(dependencies
        INTERFACE cspice
        INTERFACE csupport)

add_executable(gate main.c topo.c topo.h)
target_link_libraries(gate
        PRIVATE dependencies
        PRIVATE m)