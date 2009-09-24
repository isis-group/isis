# the ISIS library module

cmake_policy(SET CMP0005 OLD) #make it not complain about escaped Preprocessor definition (can be removed when we switch to cmake 2.6)
add_definitions(-DBUILD_PATH=\\"${PROJECT_BINARY_DIR}\\")

add_subdirectory(CoreUtils)
add_subdirectory(DataStorage)
# add_subdirectory(ExternalLibraryAdapter)
add_subdirectory(ImageIO)
