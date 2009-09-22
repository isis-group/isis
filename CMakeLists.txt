cmake_minimum_required(VERSION 2.4)

# the ISIS project
project(isis)

# add path for new modules
# set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH}
# 	"${CMAKE_SOURCE_DIR}/cmake/Modules")

add_definitions(-D_ENABLE_DATA_LOG=1)
add_definitions(-D_ENABLE_CORE_LOG=1)
add_definitions(-D_ENABLE_DATA_DEBUG=1)
add_definitions(-D_ENABLE_CORE_DEBUG=1)

include_directories(lib)

add_subdirectory(lib)
add_subdirectory(tests)
