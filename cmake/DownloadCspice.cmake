# Utility CMake file to download the CSPICE library

cmake_minimum_required(VERSION 3.13)

include(FetchContent)
FetchContent_Declare(
        cspice
        URL http://naif.jpl.nasa.gov/pub/naif/toolkit/C/PC_Linux_GCC_64bit/packages/cspice.tar.Z
        URL_HASH SHA256=93cd4fbce5818f8b7fecf3914c5756b8d41fd5bdaaeac1f4037b5a5410bc4768
)
FetchContent_Populate(cspice)

FetchContent_GetProperties(cspice)
file(COPY ${cspice_SOURCE_DIR}/include/ DESTINATION ${cspice_BINARY_DIR}/include/cspice)
file(COPY ${cspice_SOURCE_DIR}/lib/ DESTINATION ${cspice_BINARY_DIR}/lib/)

add_library(cspice_headers STATIC IMPORTED)
set_property(TARGET cspice_headers PROPERTY IMPORTED_LOCATION
        "${cspice_BINARY_DIR}/lib/cspice.a")
target_include_directories(cspice_headers INTERFACE ${cspice_BINARY_DIR}/include)

add_library(cspice_support STATIC IMPORTED)
set_property(TARGET cspice_support PROPERTY IMPORTED_LOCATION
        "${cspice_BINARY_DIR}/lib/csupport.a")

add_library(cspice INTERFACE)
target_link_libraries(cspice
        INTERFACE cspice_headers
        INTERFACE cspice_support)
