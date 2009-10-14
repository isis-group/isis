# the ISIS library module

cmake_policy(SET CMP0005 OLD) #make it not complain about escaped Preprocessor definition (can be removed when we switch to cmake 2.6)
add_definitions(-DBUILD_PATH=\\"${PROJECT_BINARY_DIR}\\")

IF(BUILD_COREUTILS)
add_subdirectory(CoreUtils)
ENDIF(BUILD_COREUTILS)

IF(BUILD_DATA_IO)
add_subdirectory(DataStorage)
add_subdirectory(ImageIO)
ENDIF(BUILD_DATA_IO)

# add_subdirectory(ExternalLibraryAdapter)
