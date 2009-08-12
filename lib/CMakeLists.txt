cmake_minimum_required(VERSION 2.4)

project(libisis)

include_directories(.)

#add definitions, compiler switches, etc.
#add_definitions(-Wall)
add_definitions(-D_ENABLE_DATA_LOG=1)
add_definitions(-D_ENABLE_CORE_LOG=1)

add_subdirectory(CoreUtils)
add_subdirectory(DataStorage)
#add_subdirectory(ImageIO)
add_subdirectory(extRegistration)
