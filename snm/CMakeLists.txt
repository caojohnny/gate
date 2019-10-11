cmake_minimum_required(VERSION 3.13)
project(gatesnm VERSION 1.0 LANGUAGES C)

add_library(gatesnm snm.c snm.h)
target_include_directories(gatesnm
        PUBLIC "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>"
        PUBLIC "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>")

write_basic_package_version_file(
        gatesnmConfigVersion.cmake
        VERSION ${PACKAGE_VERSION}
        COMPATIBILITY AnyNewerVersion)

# Exported config
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/gatesnmConfig.cmake.in"
        gatesnmConfig.cmake
        @ONLY)

# Install CMake config, version config
set(INSTALL_CMAKE_DIR ${CMAKE_INSTALL_LIBDIR}/cmake/gatesnm)
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/gatesnmConfig.cmake"
        "${CMAKE_CURRENT_BINARY_DIR}/gatesnmConfigVersion.cmake"
        DESTINATION "${INSTALL_CMAKE_DIR}")

# Install actual library targets
include(GNUInstallDirs)
install(TARGETS gatesnm
        EXPORT gatesnmTargets
        ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
        LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
        RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}")
install(EXPORT gatesnmTargets
        FILE gatesnmTargets.cmake
        DESTINATION "${INSTALL_CMAKE_DIR}")
install(DIRECTORY "${CMAKE_CURRENT_LIST_DIR}/"
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/gatesnm
        FILES_MATCHING PATTERN "*.h*")
