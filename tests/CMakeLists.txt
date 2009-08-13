cmake_minimum_required(VERSION 2.4)

add_definitions(-D_ENABLE_DATA_LOG=1)
add_definitions(-D_ENABLE_CORE_LOG=1)
add_definitions(-D_ENABLE_DATA_DEBUG=1)
add_definitions(-D_ENABLE_CORE_DEBUG=1)

set(ISIS_LIB_DIR "../lib" CACHE PATH "Path to the isis-library-path")
include_directories(${ISIS_LIB_DIR})
find_library(utils PATHS "../build/lib")
find_library(data PATHS "../build/lib")

add_executable(test_logger		test_logger.cpp)
add_executable(test_property	test_property.cpp)
add_executable(test_data		test_data.cpp)

target_link_libraries(test_logger ${utils})
target_link_libraries(test_property ${utils})
target_link_libraries(test_data ${utils} ${data})
