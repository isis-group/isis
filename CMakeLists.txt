############################################################
# The ISIS project
#
# Main CMake configuration file of the ISIS project.
#
# Author: Thomas Proeger <thomasproeger@googlemail.com>
# Date: Tue, 05 Apr 2011 10:50:31 +0200
#
############################################################

cmake_minimum_required(VERSION 2.6)

include(InstallRequiredSystemLibraries) #http://docdb.fnal.gov/ILC/sid/stanitz/ilcsoft-latest/cmake-2.4.8/Modules/InstallRequiredSystemLibraries.cmake

project(ISIS)

############################################################
# set isis version number
############################################################
set(ISIS_VERSION_MAJOR "0")
set(ISIS_VERSION_MINOR "4")
set(ISIS_VERSION_PATCH "0")

add_definitions(-D_ISIS_VERSION_MAJOR=${ISIS_VERSION_MAJOR})
add_definitions(-D_ISIS_VERSION_MINOR=${ISIS_VERSION_MINOR})
add_definitions(-D_ISIS_VERSION_PATCH=${ISIS_VERSION_PATCH})

set(ISIS_VERSION
  ${ISIS_VERSION_MAJOR}.${ISIS_VERSION_MINOR}.${ISIS_VERSION_PATCH}
  CACHE INTERNAL "The current Version of the ISIS project")

############################################################
# set install infix for plugin installation path
############################################################
set(ISIS_PLUGIN_INFIX lib/isis/plugins)

############################################################
# set project specific module path
############################################################
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH}
  "${CMAKE_CURRENT_SOURCE_DIR}/cmake/Modules")

############################################################
# general compiler flags
############################################################
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra" )
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra"  )

############################################################
# compiler flags on 64Bit cpus
############################################################
if("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "x86_64")
  message(STATUS "Compiling on x86_64. Using the '-fpic' option in any case")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC" )
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC"  )
endif("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "x86_64" )

############################################################
# Logging options are global for all components using the
# core library.
############################################################
option(ISIS_RUNTIME_LOG "Enable runtime logging" ON)
option(ISIS_DEBUG_LOG "Enable debug logging" ON)

# enable RUNTIME_LOG
if(ISIS_RUNTIME_LOG)
  add_definitions(-D_ENABLE_LOG=1)
else(ISIS_RUNTIME_LOG)
  add_definitions(-D_ENABLE_LOG=0)
endif(ISIS_RUNTIME_LOG)

# enable DEBUG_LOG
if(ISIS_DEBUG_LOG)
  add_definitions(-D_ENABLE_DEBUG=1)
  message(STATUS "Debug code enabled, this will will have a big impact on the performance")
else(ISIS_DEBUG_LOG)
  add_definitions(-D_ENABLE_DEBUG=0)
endif(ISIS_DEBUG_LOG)

# since ISIS stongly depends on the boost libraries we will configure them
# globally.
find_package(Boost REQUIRED COMPONENTS filesystem regex system date_time)
include_directories(${Boost_INCLUDE_DIR})

############################################################
# RCS revision number
############################################################
# The revision number is based on GIT commit informations.
# It will be stored in GIT_REVISION.
find_program(GIT_COMMAND git DOC "The rcs revision description information tool.")

if (NOT GIT_REVISION)
  if (GIT_COMMAND)
    execute_process(COMMAND ${GIT_COMMAND} describe OUTPUT_VARIABLE OUT WORKING_DIRECTORY ${CMAKE_SOURCE_DIR} OUTPUT_STRIP_TRAILING_WHITESPACE)
    string(REGEX MATCH "[0-9]+-[^-]+$" GIT_REVISION "${OUT}") #@todo wont work with more complex revisions/modified sources
    set(CPACK_PACKAGE_FILE_NAME ${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH}-${GIT_REVISION}-${CMAKE_SYSTEM_NAME}-${CMAKE_SYSTEM_PROCESSOR}-${CMAKE_BUILD_TYPE})
  else(GIT_COMMAND)
    message(STATUS "git not found")
  endif (GIT_COMMAND)
endif (NOT GIT_REVISION)

if (GIT_REVISION)
  message(STATUS "RCS Revision: ${GIT_REVISION}")
  add_definitions(-DISIS_RCS_REVISION=${GIT_REVISION})
  set(ISIS_CORE_REV_STRING ${${CMAKE_PROJECT_NAME}_VERSION_MAJOR}.${${CMAKE_PROJECT_NAME}_VERSION_MINOR}.${${CMAKE_PROJECT_NAME}_VERSION_PATCH}-${GIT_REVISION})
else(GIT_REVISION)
  set(ISIS_CORE_REV_STRING ${${CMAKE_PROJECT_NAME}_VERSION_MAJOR}.${${CMAKE_PROJECT_NAME}_VERSION_MINOR}.${${CMAKE_PROJECT_NAME}_VERSION_PATCH})
endif(GIT_REVISION)

############################################################
# optional components
############################################################
option(ISIS_BUILD_TESTS "Build the ISIS test cases. They depend on Boost-Test" OFF)
option(ISIS_BUILD_TOOLS "Build the ISIS command line tools (converter, diff, etc pp." OFF)
option(ISIS_BUILD_PYTHON_EXT "Build the ISIS python extension. It provides access to the ISIS core image functions from python" OFF)

############################################################
# set up doxygen
############################################################
include(UseDoxygen)

###########################################################
# CPack config
###########################################################
SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY "isis core library")
SET(CPACK_PACKAGE_VENDOR "the friendly isis team")
SET(CPACK_PACKAGE_CONTACT "reimer@cbs.mpg.de")
SET(CPACK_PACKAGE_NAME "isis-core")
SET(CPACK_BUNDLE_NAME ${CPACK_PACKAGE_NAME})
SET(CPACK_COMPONENTS_ALL "CLI Tools" "IO plugins" "Development" "RuntimeLibraries")
#SET(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_CURRENT_SOURCE_DIR}/ReadMe.txt")
SET(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/COPYING.txt")
SET(CPACK_PACKAGE_VERSION_MAJOR ${${CMAKE_PROJECT_NAME}_VERSION_MAJOR})
SET(CPACK_PACKAGE_VERSION_MINOR ${${CMAKE_PROJECT_NAME}_VERSION_MINOR})
SET(CPACK_PACKAGE_VERSION_PATCH ${${CMAKE_PROJECT_NAME}_VERSION_PATCH})
SET(CPACK_PACKAGE_FILE_NAME ${CPACK_PACKAGE_NAME}-${ISIS_CORE_REV_STRING}-${CMAKE_SYSTEM_NAME}-${CMAKE_SYSTEM_PROCESSOR}-${CMAKE_BUILD_TYPE})
#SET(CPACK_PACKAGE_INSTALL_DIRECTORY "CMake ${${CMAKE_PROJECT_NAME}_VERSION_MAJOR}.${${CMAKE_PROJECT_NAME}_VERSION_MINOR}")
INCLUDE(CPack)

# status messages
message(STATUS "/bin directory: ${CMAKE_INSTALL_PREFIX}/bin")
message(STATUS "/lib directory: ${CMAKE_INSTALL_PREFIX}/lib")
message(STATUS "/include directory: ${CMAKE_INSTALL_PREFIX}/include")
message(STATUS "/plugins directory: ${CMAKE_INSTALL_PREFIX}/${ISIS_PLUGIN_INFIX}")

# build the ISIS libraries
add_subdirectory(lib)

# build the ISIS applications
add_subdirectory(apps)

# configure coding style tools
add_subdirectory(design)

# build the ISIS tools.
if(ISIS_BUILD_TOOLS)
  add_subdirectory(tools)
endif(ISIS_BUILD_TOOLS)

# build the ISIS tests.
if(ISIS_BUILD_TESTS)
  add_subdirectory(tests)
endif(ISIS_BUILD_TESTS)

# build the ISIS python extension.
if(ISIS_BUILD_PYTHON_EXT)
  add_subdirectory(python)
endif(ISIS_BUILD_PYTHON_EXT)

# uninstall target
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/cmake//cmake_uninstall.cmake.in" "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake" IMMEDIATE @ONLY)
add_custom_target(uninstall COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake)
