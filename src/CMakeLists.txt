cmake_minimum_required(VERSION 3.13)
project(libgate VERSION 1.0 LANGUAGES C)

include(CMakePackageConfigHelpers)
include("${CMAKE_SOURCE_DIR}/cmake/DownloadCspice.cmake")
include("${CMAKE_SOURCE_DIR}/cmake/Doxygen.cmake")

add_library(gate
        topo.h topo.c
        stars.c stars.h
        timeconv.c timeconv.h
        constants.h)
target_include_directories(gate
        PUBLIC "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>"
        PUBLIC "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>")
target_link_libraries(gate
        PUBLIC cspice
        PRIVATE m)

# Version config
write_basic_package_version_file(
        gateConfigVersion.cmake
        VERSION ${PACKAGE_VERSION}
        COMPATIBILITY AnyNewerVersion)

# Exported config
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/gateConfig.cmake.in"
        gateConfig.cmake
        @ONLY)

# Install CMake config, version config
set(INSTALL_CMAKE_DIR ${CMAKE_INSTALL_LIBDIR}/cmake/gate)
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/gateConfig.cmake"
        "${CMAKE_CURRENT_BINARY_DIR}/gateConfigVersion.cmake"
        DESTINATION "${INSTALL_CMAKE_DIR}")

# Install actual library targets
include(GNUInstallDirs)
install(TARGETS gate
        EXPORT gateTargets
        ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
        LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
        RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}")
install(EXPORT gateTargets
        FILE gateTargets.cmake
        DESTINATION "${INSTALL_CMAKE_DIR}")
install(DIRECTORY "${CMAKE_CURRENT_LIST_DIR}/"
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/gate
        FILES_MATCHING PATTERN "*.h*")