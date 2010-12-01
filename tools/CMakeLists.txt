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
    "The directory where the isis cmake configuration script is located.")

FIND_PACKAGE(ISIS REQUIRED)
INCLUDE_DIRECTORIES(${ISIS_INCLUDE_DIRS})

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
target_link_libraries(isisdump ${ISIS_LIB} ${ISIS_LIB_DEPENDS})
SET_TARGET_PROPERTIES(isisdump PROPERTIES INSTALL_RPATH_USE_LINK_PATH TRUE)

add_executable(isisdiff isisdiff.cpp)
target_link_libraries(isisdiff ${ISIS_LIB} ${ISIS_LIB_DEPENDS})
SET_TARGET_PROPERTIES(isisdiff PROPERTIES INSTALL_RPATH_USE_LINK_PATH TRUE)

add_executable(isisconv isisconv.cpp)
target_link_libraries(isisconv ${ISIS_LIB} ${ISIS_LIB_DEPENDS})
SET_TARGET_PROPERTIES(isisconv PROPERTIES INSTALL_RPATH_USE_LINK_PATH TRUE)

add_executable(isisswap isisswap.cpp)
target_link_libraries(isisswap ${ISIS_LIB} ${ISIS_LIB_DEPENDS})
SET_TARGET_PROPERTIES(isisswap PROPERTIES INSTALL_RPATH_USE_LINK_PATH TRUE)

###########################################################
# CPack config
###########################################################
SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY "isis cli tools")
SET(CPACK_PACKAGE_VENDOR "the friendly isis team")
SET(CPACK_PACKAGE_CONTACT "reimer@cbs.mpg.de")
SET(CPACK_PACKAGE_NAME "isis-tools")
SET(CPACK_BUNDLE_NAME ${CPACK_PACKAGE_NAME})
#SET(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_CURRENT_SOURCE_DIR}/ReadMe.txt")
SET(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/../COPYING.txt")
SET(CPACK_PACKAGE_VERSION_MAJOR ${${CMAKE_PROJECT_NAME}_VERSION_MAJOR})
SET(CPACK_PACKAGE_VERSION_MINOR ${${CMAKE_PROJECT_NAME}_VERSION_MINOR})
SET(CPACK_PACKAGE_VERSION_PATCH ${${CMAKE_PROJECT_NAME}_VERSION_PATCH})
SET(CPACK_PACKAGE_FILE_NAME ${CPACK_PACKAGE_NAME}-${ISIS_CORE_REV_STRING}-${CMAKE_SYSTEM_NAME}-${CMAKE_SYSTEM_PROCESSOR}-${CMAKE_BUILD_TYPE})
#SET(CPACK_PACKAGE_INSTALL_DIRECTORY "CMake ${${CMAKE_PROJECT_NAME}_VERSION_MAJOR}.${${CMAKE_PROJECT_NAME}_VERSION_MINOR}")
INCLUDE(CPack)

# the install targets.
INSTALL(TARGETS
    isisdump
    isisdiff
    isisconv
    isisswap
    RUNTIME DESTINATION bin)
    
MESSAGE(STATUS "Installing binary files to ${CMAKE_INSTALL_PREFIX}/bin")

