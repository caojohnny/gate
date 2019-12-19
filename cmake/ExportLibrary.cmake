# Exports a library in standard structure format
# along with any associated files

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)
set(EL_MODULE_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
set(EL_STAGING_DIR "${CMAKE_CURRENT_BINARY_DIR}")
set(EL_INCLUDE_DIR "${CMAKE_INSTALL_INCLUDEDIR}")
set(EL_LIBRARY_DIR "${CMAKE_INSTALL_LIBDIR}")
set(EL_META_DIR "${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}")
set(EL_LIBRARY_NAME "${PROJECT_NAME}")
set(EL_LIBRARY_VERSION "${PACKAGE_VERSION}")

set(EL_VERSION_CONFIG_FILE "${EL_STAGING_DIR}/${EL_LIBRARY_NAME}ConfigVersion.cmake")
set(EL_CONFIG_NAME "${EL_LIBRARY_NAME}Config.cmake")
set(EL_CONFIG_IN_NAME "${EL_CONFIG_NAME}.in")
set(EL_TARGETS_NAME "${EL_LIBRARY_NAME}Targets")
set(EL_TARGETS_IN_NAME "${EL_TARGETS_NAME}.cmake")

# Version config
write_basic_package_version_file(
        "${EL_VERSION_CONFIG_FILE}"
        VERSION "${EL_LIBRARY_VERSION}"
        COMPATIBILITY AnyNewerVersion)

# Exported config
configure_file("${EL_MODULE_DIR}/${EL_CONFIG_IN_NAME}"
        "${EL_STAGING_DIR}/${EL_CONFIG_NAME}"
        @ONLY)

# Install CMake config, version config
install(FILES "${EL_STAGING_DIR}/${EL_CONFIG_NAME}"
        "${EL_VERSION_CONFIG_FILE}"
        DESTINATION "${EL_META_DIR}")

# Export library binaries
install(TARGETS "${EL_LIBRARY_NAME}"
        EXPORT "${EL_TARGETS_NAME}"
        ARCHIVE DESTINATION "${EL_LIBRARY_DIR}"
        LIBRARY DESTINATION "${EL_LIBRARY_DIR}")
# Export target CMake files
install(EXPORT "${EL_TARGETS_NAME}"
        FILE "${EL_TARGETS_IN_NAME}"
        DESTINATION "${EL_META_DIR}")

# Export project include files
install(DIRECTORY "${EL_MODULE_DIR}/${EL_LIBRARY_NAME}/"
        DESTINATION "${EL_INCLUDE_DIR}/${EL_LIBRARY_NAME}"
        FILES_MATCHING PATTERN "*.h*")

