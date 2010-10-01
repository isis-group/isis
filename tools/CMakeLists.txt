# The ISIS tools project
#
# The isis tools are: 
# 
# isisdump    -    Dumps the meta data informations from an image file as seen
#                  by the ISIS internal data representation.
# isisiff     -    Produces a difference output between the meta information
#                  given by two image files A and B.
# isisconv    -    The ISIS data converter. It converts image data between all
#                  different formats provided by the imageIO plugins
# isisswap    -    Swaps the image orientation and voxel data along a given axis.

cmake_minimum_required(VERSION 2.6)

project(ISIS_TOOLS)

###########################################################
# find and configure the isis core library
###########################################################
SET(ISIS_DIR /usr/share/isis/cmake CACHE PATH 
    "The directory where the isis cmake configuration skript script is located.")
FIND_PACKAGE(ISIS REQUIRED)
INCLUDE_DIRECTORIES(${ISIS_INCLUDE_DIR})
LINK_DIRECTORIES(${ISIS_LIB_DIR})
# copy definitions for debug and runtime log
# enable RUNTIME_LOG
IF(${CMAKE_PROJECT_NAME}_RUNTIME_LOG)
	ADD_DEFINITIONS(-D_ENABLE_LOG=1)
ELSE(${CMAKE_PROJECT_NAME}_RUNTIME_LOG)
	ADD_DEFINITIONS(-D_ENABLE_LOG=0)
ENDIF(${CMAKE_PROJECT_NAME}_RUNTIME_LOG)
# enable DEBUG_LOG
IF(${CMAKE_PROJECT_NAME}_DEBUG_LOG)
	ADD_DEFINITIONS(-D_ENABLE_DEBUG=1)
	MESSAGE(STATUS "Debug code enabled, this will will have a big impact on the performance")
ELSE(${CMAKE_PROJECT_NAME}_DEBUG_LOG)
	ADD_DEFINITIONS(-D_ENABLE_DEBUG=0)
ENDIF(${CMAKE_PROJECT_NAME}_DEBUG_LOG)

add_executable(isisdump isisdump.cpp)
target_link_libraries(isisdump ${ISIS_LIB})

add_executable(isisdiff isisdiff.cpp)
target_link_libraries(isisdiff ${ISIS_LIB})

add_executable(isisconv isisconv.cpp)
target_link_libraries(isisconv ${ISIS_LIB})

add_executable(isisswap isisswap.cpp)
target_link_libraries(isisswap ${ISIS_LIB})
