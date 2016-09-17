############################################################
# The ISIS project
#
# Main CMake configuration file of the ISIS project.
#
# Author: Thomas Proeger <thomasproeger@googlemail.com>
# Date: Tue, 05 Apr 2011 10:50:31 +0200
#
############################################################
cmake_minimum_required(VERSION 2.8)
find_package(Git)

if(CMAKE_MAJOR_VERSION EQUAL 3)
cmake_policy(SET CMP0042 NEW)
cmake_policy(SET CMP0048 OLD)
endif(CMAKE_MAJOR_VERSION EQUAL 3)

include(InstallRequiredSystemLibraries) #http://docdb.fnal.gov/ILC/sid/stanitz/ilcsoft-latest/cmake-2.4.8/Modules/InstallRequiredSystemLibraries.cmake
include(CheckIncludeFiles)

project(ISIS)

option(BUILD_TESTING OFF)
include(CTest)

############################################################
# set isis version number
############################################################
set(ISIS_VERSION_MAJOR "0")
set(ISIS_VERSION_MINOR "7")
set(ISIS_VERSION_PATCH "9")

add_definitions(-D_ISIS_VERSION_MAJOR=${ISIS_VERSION_MAJOR})
add_definitions(-D_ISIS_VERSION_MINOR=${ISIS_VERSION_MINOR})
add_definitions(-D_ISIS_VERSION_PATCH=${ISIS_VERSION_PATCH})

set(ISIS_VERSION
  ${ISIS_VERSION_MAJOR}.${ISIS_VERSION_MINOR}.${ISIS_VERSION_PATCH}
  CACHE INTERNAL "The current Version of the ISIS project")

set(ISIS_CORE_VERSION_API "4.0.0")
set(ISIS_QT_VERSION_API "0.0.0")
STRING( REGEX MATCH "^[0-9]+" ISIS_QT_VERSION_SO ${ISIS_QT_VERSION_API})
STRING( REGEX MATCH "^[0-9]+" ISIS_CORE_VERSION_SO ${ISIS_CORE_VERSION_API})

############################################################
# check for byteswap intrinsics
############################################################
CHECK_INCLUDE_FILES(byteswap.h HAVE_BYTESWAP)

if(HAVE_BYTESWAP)
	add_definitions(-DHAVE_BYTESWAP)
endif(HAVE_BYTESWAP)

if(NOT WIN32)
############################################################
# set install infix for plugin installation path
############################################################
set(ISIS_PLUGIN_INFIX lib/isis/plugins)
endif(NOT WIN32)

############################################################
# set project specific module path
############################################################
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH}
  "${CMAKE_CURRENT_SOURCE_DIR}/cmake/Modules")

############################################################
# compiler flags 
############################################################

if("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "x86_64")
  message(STATUS "Compiling on x86_64. Using the '-fPIC' option in any case")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC" )
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC"  )
endif("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "x86_64" )

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=c++11" )
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall"  )


############################################################
# Logging options are global for all components in the tree
############################################################
option(ISIS_RUNTIME_LOG "Enable runtime logging" ON)
option(ISIS_DEBUG_LOG "Enable debug logging" ON)

############################################################
# optional components
############################################################
option(ISIS_BUILD_TOOLS "Build the ISIS command line tools (converter, diff, etc pp." OFF)

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
if(BUILD_TESTING)
	find_package(Boost REQUIRED COMPONENTS filesystem system unit_test_framework)
else(BUILD_TESTING)
	find_package(Boost REQUIRED COMPONENTS filesystem system)
endif(BUILD_TESTING)

include_directories(${Boost_INCLUDE_DIR})

############################################################
# Macro to generate manpages for CLI programms
############################################################
set(MANPAGES OFF CACHE BOOL "Create manpages for the tools")

if(MANPAGES)
	find_program(HELP2MAN help2man)
endif(MANPAGES)

macro( build_manpage name desc)
	if(MANPAGES)
		add_custom_command(
			TARGET ${name}
			POST_BUILD
			COMMAND ${HELP2MAN} ./${name} --version-string=${ISIS_CORE_REV_STRING} --no-discard-stderr --no-info -o ${name}.1 -s 1 --name=${desc}
			COMMENT "(Re)Generating manpage ${name}.1"
            VERBATIM
		)
		install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${name}.1 DESTINATION share/man/man1 COMPONENT "CLI_Tools")
	endif(MANPAGES)
endmacro( build_manpage )


############################################################
# RCS revision number
############################################################
# The revision number is based on GIT commit informations.
# It will be stored in GIT_REVISION.
if (NOT GIT_REVISION)
  if (GIT_FOUND)
    execute_process(COMMAND ${GIT_EXECUTABLE} describe OUTPUT_VARIABLE OUT WORKING_DIRECTORY ${CMAKE_SOURCE_DIR} OUTPUT_STRIP_TRAILING_WHITESPACE)
    string(REGEX MATCH "[0-9]+-[^-]+$" GIT_REVISION "${OUT}") #@todo wont work with more complex revisions/modified sources
    set(CPACK_PACKAGE_FILE_NAME ${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH}-${GIT_REVISION}-${CMAKE_SYSTEM_NAME}-${PROCESSOR}-${CMAKE_BUILD_TYPE})
  else(GIT_COMMAND)
    message(STATUS "git not found")
  endif (GIT_FOUND)
endif (NOT GIT_REVISION)

if (GIT_REVISION)
  message(STATUS "RCS Revision: ${GIT_REVISION}")
  add_definitions(-DISIS_RCS_REVISION=${GIT_REVISION})
  set(ISIS_CORE_REV_STRING ${ISIS_VERSION_MAJOR}.${ISIS_VERSION_MINOR}.${ISIS_VERSION_PATCH}-${GIT_REVISION})
else(GIT_REVISION)
  set(ISIS_CORE_REV_STRING ${ISIS_VERSION_MAJOR}.${ISIS_VERSION_MINOR}.${ISIS_VERSION_PATCH})
endif(GIT_REVISION)

###########################################################
# CPack config
###########################################################
SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY "isis core library")
SET(CPACK_PACKAGE_VENDOR "the friendly isis team")
SET(CPACK_PACKAGE_CONTACT "reimer@cbs.mpg.de")
SET(CPACK_PACKAGE_NAME "isis-core")
SET(CPACK_BUNDLE_NAME ${CPACK_PACKAGE_NAME})
SET(CPACK_COMPONENTS_ALL "CLI_Tools" "IO_plugins" "Development" "RuntimeLibraries")
#SET(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_CURRENT_SOURCE_DIR}/ReadMe.txt")
SET(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/COPYING.txt")
SET(CPACK_PACKAGE_VERSION_MAJOR ${ISIS_VERSION_MAJOR})
SET(CPACK_PACKAGE_VERSION_MINOR ${ISIS_VERSION_MINOR})
SET(CPACK_PACKAGE_VERSION_PATCH ${ISIS_VERSION_PATCH})
SET(CPACK_PACKAGE_FILE_NAME ${CPACK_PACKAGE_NAME}-${ISIS_CORE_REV_STRING}-${CMAKE_SYSTEM_NAME}-${PROCESSOR}-${CMAKE_BUILD_TYPE})
#SET(CPACK_PACKAGE_INSTALL_DIRECTORY "CMake ${ISIS_VERSION_MAJOR}.${ISIS_VERSION_MINOR}")
SET(CPACK_NSIS_MODIFY_PATH ON)
SET(CPACK_NSIS_URL_INFO_ABOUT "http:\\\\\\\\isis-group.github.com/isis")
INCLUDE(CPack)

###########################################################
# Doxygen target
###########################################################
option(BUILD_DOCUMENTATION "Create and install the HTML based API documentation (requires Doxygen)" false)
if(BUILD_DOCUMENTATION)
	find_package(Doxygen)
    if(NOT DOXYGEN_FOUND)
        message(FATAL_ERROR "Doxygen and latex are needed to build the documentation.")
    endif()

    set(doxyfile_in ${CMAKE_CURRENT_SOURCE_DIR}/cmake/Doxyfile.in)
    set(doxyfile ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)

    configure_file(${doxyfile_in} ${doxyfile} @ONLY)

    add_custom_target(doc
        COMMAND ${DOXYGEN_EXECUTABLE} ${doxyfile}
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Generating API documentation with Doxygen"
        VERBATIM)

#    install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/html DESTINATION share/doc)
endif()




# build the ISIS libraries
add_subdirectory(isis)

# build the ISIS applications
add_subdirectory(apps)

# configure coding style tools
add_subdirectory(design)

# configure tests
add_subdirectory(tests)

# build the ISIS tools.
if(ISIS_BUILD_TOOLS)
  add_subdirectory(tools)
endif(ISIS_BUILD_TOOLS)

add_subdirectory(io_plugins)

# uninstall target
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/cmake//cmake_uninstall.cmake.in" "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake" IMMEDIATE @ONLY)
add_custom_target(uninstall COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake)
