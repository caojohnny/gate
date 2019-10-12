# Utility CMake file to download the CSPICE library

cmake_minimum_required(VERSION 3.13)

include(FetchContent)
FetchContent_GetProperties(cspice)

if(NOT cspice_POPULATED)
    if (UNIX)
        if (NOT APPLE)
            message("Downloading CSPICE for Linux")
            FetchContent_Declare(
                    cspice
                    URL http://naif.jpl.nasa.gov/pub/naif/toolkit/C/PC_Linux_GCC_64bit/packages/cspice.tar.Z
                    URL_HASH SHA256=93cd4fbce5818f8b7fecf3914c5756b8d41fd5bdaaeac1f4037b5a5410bc4768
            )
        else ()
            message("Downloading CSPICE for Mac OSX")
            FetchContent_Declare(
                    cspice
                    URL https://naif.jpl.nasa.gov/pub/naif/toolkit//C/MacIntel_OSX_AppleC_64bit/packages/cspice.tar.Z
                    URL_HASH SHA256=f5d48c4b0d558c5d71e8bf6fcdf135b0943210c1ff91f8191dfc447419a6b12e
            )
        endif ()
    endif ()
    FetchContent_Populate(cspice)

    file(COPY "${cspice_SOURCE_DIR}/include/" DESTINATION "${cspice_BINARY_DIR}/include/cspice")
    file(COPY "${cspice_SOURCE_DIR}/lib/" DESTINATION "${cspice_BINARY_DIR}/lib/")

    add_library(cspice_headers STATIC IMPORTED)
    set_property(TARGET cspice_headers PROPERTY IMPORTED_LOCATION
            "${cspice_BINARY_DIR}/lib/cspice.a")
    target_include_directories(cspice_headers
            INTERFACE ${cspice_BINARY_DIR}/include)

    add_library(cspice_support STATIC IMPORTED)
    set_property(TARGET cspice_support PROPERTY IMPORTED_LOCATION
            "${cspice_BINARY_DIR}/lib/csupport.a")

    add_library(cspice INTERFACE IMPORTED)
    target_link_libraries(cspice
            INTERFACE cspice_headers
            INTERFACE cspice_support)
endif ()
