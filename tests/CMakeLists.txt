cmake_minimum_required(VERSION 2.4)

SET(CMAKE_C_FLAGS "-fexceptions")
SET(CMAKE_CXX_FLAGS "-fexceptions")

add_definitions(-D_ENABLE_DATA_LOG=1)
add_definitions(-D_ENABLE_CORE_LOG=1)
add_definitions(-D_ENABLE_DATA_DEBUG=1)
add_definitions(-D_ENABLE_CORE_DEBUG=1)

# set(ISIS_LIB_DIR "../libisis" CACHE PATH "Path to the isis-library-path")

find_path(ISIS_LIBRARY_DIR
    "libutils.so"
    PATHS ${CMAKE_SOURCE_DIR}/../build/lib/
    DOC "Path to the isis libraries"
    )
    
find_path(ISIS_INCLUDE_DIR
    "CoreUtils/common.hpp"
    PATHS ${CMAKE_SOURCE_DIR}/../lib
    DOC "Path to the isis header files"
    )

include_directories(${ISIS_INCLUDE_DIR})
link_directories(${ISIS_LIBRARY_DIR})

find_library(LIB_UTILS utils PATHS ${ISIS_LIBRARY_DIR})
find_library(LIB_DATA data PATHS ${ISIS_LIBRARY_DIR})

add_executable(test_logger		test_logger.cpp)
add_executable(test_property	test_property.cpp)
add_executable(test_data		test_data.cpp)

target_link_libraries(test_logger utils)
target_link_libraries(test_property utils)
target_link_libraries(test_data utils data)


