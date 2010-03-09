# the ISIS library module

SET(PLUGIN_PATH ${PROJECT_BINARY_DIR} CACHE PATH "Path where the plugins shall be installed")

add_definitions(-DPLUGIN_PATH="${PLUGIN_PATH}")
add_definitions(-DDL_SUFFIX="${CMAKE_SHARED_LIBRARY_SUFFIX}")
add_definitions(-DDL_PREFIX="${CMAKE_SHARED_LIBRARY_PREFIX}")

IF(BUILD_COREUTILS)
add_subdirectory(CoreUtils)
ENDIF(BUILD_COREUTILS)

IF(BUILD_DATA_IO)
add_subdirectory(DataStorage)
add_subdirectory(ImageIO)
ENDIF(BUILD_DATA_IO)

IF(HAVE_ITK)
add_subdirectory(extITK)
ENDIF(HAVE_ITK)

# add_subdirectory(ExternalLibraryAdapter)
