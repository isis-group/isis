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

FIND_PACKAGE(Boost COMPONENTS filesystem regex system)
include_directories(${Boost_INCLUDE_DIR}  lib)


add_subdirectory(lib)


OPTION(BUILD_APPLICATIONS "Build the isis application directory." OFF)
OPTION(BUILD_TESTS "Build the isis tests directory." ON)
OPTION(BUILD_SHARED_LIBS "Build isis with shared libraries" OFF)

IF(BUILD_APPLICATIONS)
	add_subdirectory(apps)
ENDIF(BUILD_APPLICATIONS)

IF(BUILD_TESTS)
    add_subdirectory(tests)
ENDIF(BUILD_TESTS)

SET(ISIS_INSTALL_BIN_DIR "${CMAKE_BINARY_DIR}/bin/")