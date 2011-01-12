# the ISIS core module
CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

INCLUDE(InstallRequiredSystemLibraries)

PROJECT(ISIS_CORE)

############################################################
# set isis version number
############################################################
SET(${CMAKE_PROJECT_NAME}_VERSION_MAJOR "0")
SET(${CMAKE_PROJECT_NAME}_VERSION_MINOR "2")
SET(${CMAKE_PROJECT_NAME}_VERSION_PATCH "0")

ADD_DEFINITIONS(-D_ISIS_VERSION_MAJOR=${${CMAKE_PROJECT_NAME}_VERSION_MAJOR})
ADD_DEFINITIONS(-D_ISIS_VERSION_MINOR=${${CMAKE_PROJECT_NAME}_VERSION_MINOR})
ADD_DEFINITIONS(-D_ISIS_VERSION_PATCH=${${CMAKE_PROJECT_NAME}_VERSION_PATCH})

SET(${CMAKE_PROJECT_NAME}_VERSION
  ${${CMAKE_PROJECT_NAME}_VERSION_MAJOR}.${${CMAKE_PROJECT_NAME}_VERSION_MINOR}.${${CMAKE_PROJECT_NAME}_VERSION_PATCH})

############################################################
# Set ISIS install paths
############################################################
SET(ISIS_PLUGIN_INFIX share/isis/plugins)

############################################################
# check for liboil
############################################################
set(ISIS_USE_LIBOIL OFF CACHE BOOL "use liboil" )
if(ISIS_USE_LIBOIL)
	find_path(LibOil_INCLUDE_DIR "liboil/liboil.h" PATH_SUFFIXES "liboil" "liboil-0.3")
	find_library(LibOil_LIBRARY NAMES "oil-0.3")
	include_directories(${LibOil_INCLUDE_DIR})
	ADD_DEFINITIONS(-DISIS_USE_LIBOIL)
else(ISIS_USE_LIBOIL)
	set(LibOil_LIBRARY "")
endif(ISIS_USE_LIBOIL)

############################################################
# set project specific module path
############################################################
SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH}
  "${CMAKE_CURRENT_SOURCE_DIR}/cmake/Modules")

############################################################
# set up doxygen
############################################################
include(UseDoxygen)

############################################################
# Compiler flags on 64Bit cpus
############################################################
IF("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "x86_64")
  MESSAGE(STATUS "Compiling on x86_64. Using the '-fpic' option in any case")
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC" )
    SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC"  )
ENDIF("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "x86_64" )

############################################################
# LOGGING
############################################################
OPTION(${CMAKE_PROJECT_NAME}_RUNTIME_LOG "Enable runtime logging" ON)
OPTION(${CMAKE_PROJECT_NAME}_DEBUG_LOG "Enable debug logging" ON)

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

# Search BOOST packages
FIND_PACKAGE(Boost REQUIRED COMPONENTS filesystem regex system date_time)
INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIR})

# local include directory
INCLUDE_DIRECTORIES(.)

############################################################
# set the plugin directory and the system dependend shared
# library naming conventions
############################################################
ADD_DEFINITIONS(-DPLUGIN_PATH="${CMAKE_INSTALL_PREFIX}/${ISIS_PLUGIN_INFIX}")
ADD_DEFINITIONS(-DDL_SUFFIX="${CMAKE_SHARED_LIBRARY_SUFFIX}")
ADD_DEFINITIONS(-DDL_PREFIX="${CMAKE_SHARED_LIBRARY_PREFIX}")

############################################################
# RCS revision number
############################################################
# Set the revision number if this dir is a svn working copy.
# The revision number will be stored in GIT_REVISION.
find_program(GIT_COMMAND git DOC "The rcs revision description information tool.")

IF (NOT GIT_REVISION)
	IF (GIT_COMMAND)
		EXECUTE_PROCESS(COMMAND ${GIT_COMMAND} describe OUTPUT_VARIABLE OUT WORKING_DIRECTORY ${CMAKE_SOURCE_DIR} OUTPUT_STRIP_TRAILING_WHITESPACE)
		string(REGEX MATCH "[0-9]+-[^-]+$" GIT_REVISION "${OUT}") #@todo wont work with more complex revisions/modified sources
		SET(CPACK_PACKAGE_FILE_NAME ${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH}-${GIT_REVISION}-${CMAKE_SYSTEM_NAME}-${CMAKE_SYSTEM_PROCESSOR}-${CMAKE_BUILD_TYPE})
	else(GIT_COMMAND)
		message(STATUS "git not found")
	ENDIF (GIT_COMMAND)
ENDIF (NOT GIT_REVISION)

IF (GIT_REVISION)
  MESSAGE(STATUS "RCS Revision: ${GIT_REVISION}")
  ADD_DEFINITIONS(-DISIS_RCS_REVISION=${GIT_REVISION})
  SET(ISIS_CORE_REV_STRING ${${CMAKE_PROJECT_NAME}_VERSION_MAJOR}.${${CMAKE_PROJECT_NAME}_VERSION_MINOR}.${${CMAKE_PROJECT_NAME}_VERSION_PATCH}-${GIT_REVISION})
else(GIT_REVISION)
  SET(ISIS_CORE_REV_STRING ${${CMAKE_PROJECT_NAME}_VERSION_MAJOR}.${${CMAKE_PROJECT_NAME}_VERSION_MINOR}.${${CMAKE_PROJECT_NAME}_VERSION_PATCH})
ENDIF(GIT_REVISION)

###########################################################
# CPack config
###########################################################
SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY "isis core library")
SET(CPACK_PACKAGE_VENDOR "the friendly isis team")
SET(CPACK_PACKAGE_CONTACT "reimer@cbs.mpg.de")
SET(CPACK_PACKAGE_NAME "isis-core")
SET(CPACK_BUNDLE_NAME ${CPACK_PACKAGE_NAME})
#SET(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_CURRENT_SOURCE_DIR}/ReadMe.txt")
SET(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/../../COPYING.txt")
SET(CPACK_PACKAGE_VERSION_MAJOR ${${CMAKE_PROJECT_NAME}_VERSION_MAJOR})
SET(CPACK_PACKAGE_VERSION_MINOR ${${CMAKE_PROJECT_NAME}_VERSION_MINOR})
SET(CPACK_PACKAGE_VERSION_PATCH ${${CMAKE_PROJECT_NAME}_VERSION_PATCH})
SET(CPACK_PACKAGE_FILE_NAME ${CPACK_PACKAGE_NAME}-${ISIS_CORE_REV_STRING}-${CMAKE_SYSTEM_NAME}-${CMAKE_SYSTEM_PROCESSOR}-${CMAKE_BUILD_TYPE})
#SET(CPACK_PACKAGE_INSTALL_DIRECTORY "CMake ${${CMAKE_PROJECT_NAME}_VERSION_MAJOR}.${${CMAKE_PROJECT_NAME}_VERSION_MINOR}")
INCLUDE(CPack)

############################################################
# Build config
############################################################
SET(ISIS_BUILD_PROPERTIES LINK_FLAGS "--no-undefined" CLEAN_DIRECT_OUTPUT 1)
SET(ISIS_BUILD_STATIC ON CACHE BOOL "enable build of the static library" )
SET(ISIS_BUILD_DYNAMIC ON CACHE BOOL "enable build of the dynamic library" )


############################################################
# PACKAGING
# @TODO: Check if CPACK is a better solution for building 
# a rpm package
############################################################
OPTION(${CMAKE_PROJECT_NAME}_PACKAGE_RPM 
    "Enable support for RPM packaging" OFF)

IF(${CMAKE_PROJECT_NAME}_PACKAGE_RPM)

    MESSAGE(STATUS "Configuring RPM build")

    # write spec file for rpm package
    CONFIGURE_FILE(cmake/template.spec.in ${CMAKE_BINARY_DIR}/libisiscore0.spec
      @ONLY)
    CONFIGURE_FILE(cmake/bootstrap_rpm.sh.in ${CMAKE_BINARY_DIR}/bootstrap_rpm.sh
      @ONLY)

    # add a custom make target
    ADD_CUSTOM_TARGET(rpm   
        "${CMAKE_BINARY_DIR}/bootstrap_rpm.sh" WORKING_DIRECTORY ${CMAKE_BINARY_DIR})

ENDIF()

############################################################
# build core
############################################################

# add new source files to this parameter list
SET ( SRC_FILES
  CoreUtils/istring.cpp
  CoreUtils/progressfeedback.cpp 
  CoreUtils/tmpfile.cpp 
  CoreUtils/application.cpp 
  CoreUtils/selection.cpp 
  CoreUtils/progparameter.cpp 
  CoreUtils/type_converter.cpp 
  CoreUtils/type_base.cpp 
  CoreUtils/types.cpp 
  CoreUtils/property.cpp 
  CoreUtils/propmap.cpp 
  CoreUtils/message.cpp 
  CoreUtils/singletons.cpp 
  DataStorage/sortedchunklist.cpp
  DataStorage/io_application.cpp 
  DataStorage/needed.cpp 
  DataStorage/chunk.cpp 
  DataStorage/image.cpp 
  DataStorage/io_factory.cpp 
  DataStorage/typeptr_converter.cpp 
  DataStorage/numeric_convert.cpp 
  DataStorage/io_interface.cpp 
  DataStorage/typeptr_base.cpp 
  DataStorage/common.cpp )

SET (COREUTILS_HDR_FILES
  ./CoreUtils/istring.hpp
  ./CoreUtils/tmpfile.h
  ./CoreUtils/progparameter.hpp
  ./CoreUtils/type_converter.hpp
  ./CoreUtils/progressfeedback.hpp
  ./CoreUtils/application.hpp
  ./CoreUtils/singletons.hpp
  ./CoreUtils/log_modules.hpp
  ./CoreUtils/propmap.hpp
  ./CoreUtils/type_base.hpp
  ./CoreUtils/types.hpp
  ./CoreUtils/log.hpp
  ./CoreUtils/selection.hpp
  ./CoreUtils/generic_type.hpp
  ./CoreUtils/vector.hpp
  ./CoreUtils/property.hpp
  ./CoreUtils/type.hpp
  ./CoreUtils/common.hpp
  ./CoreUtils/message.hpp
  ./CoreUtils/color.hpp)

SET (DATASTORAGE_HDR_FILES
  ./DataStorage/chunk.hpp
  ./DataStorage/sortedchunklist.hpp
  ./DataStorage/io_factory.hpp
  ./DataStorage/typeptr.hpp
  ./DataStorage/numeric_convert.hpp
  ./DataStorage/typeptr_converter.hpp
  ./DataStorage/image.hpp
  ./DataStorage/io_application.hpp
  ./DataStorage/typeptr_base.hpp
  ./DataStorage/ndimensional.hpp
  ./DataStorage/io_interface.h
  ./DataStorage/common.hpp )

# the core library shared and static build
if(ISIS_BUILD_DYNAMIC)
	SET(LIBS isis_core ${LIBS})
	ADD_LIBRARY( isis_core SHARED ${SRC_FILES} )
	TARGET_LINK_LIBRARIES( isis_core ${CMAKE_DL_LIBS} ${Boost_LIBRARIES} ${LibOil_LIBRARY})
	SET_TARGET_PROPERTIES( isis_core PROPERTIES	${ISIS_BUILD_PROPERTIES} VERSION ${${CMAKE_PROJECT_NAME}_VERSION} INSTALL_NAME_DIR "${CMAKE_INSTALL_PREFIX}/lib")
endif(ISIS_BUILD_DYNAMIC)

if(ISIS_BUILD_STATIC)
	SET(LIBS isis_static ${LIBS})
	ADD_LIBRARY( isis_static STATIC ${SRC_FILES} )
	SET_TARGET_PROPERTIES( isis_static PROPERTIES ${ISIS_BUILD_PROPERTIES} OUTPUT_NAME "isis_core" )
endif(ISIS_BUILD_STATIC)

############################################################
# Installation
############################################################

# install libraries
INSTALL (TARGETS ${LIBS}
  RUNTIME COMPONENT RuntimeLibraries
  LIBRARY DESTINATION lib COMPONENT RuntimeLibraries
  ARCHIVE DESTINATION lib COMPONENT Development)

# install header files
INSTALL(FILES ${COREUTILS_HDR_FILES} DESTINATION include/isis/CoreUtils COMPONENT Development)
INSTALL(FILES ${DATASTORAGE_HDR_FILES} DESTINATION include/isis/DataStorage COMPONENT Development)

# status messages
MESSAGE(STATUS "/lib directory: ${CMAKE_INSTALL_PREFIX}/lib")
MESSAGE(STATUS "/include directory: ${CMAKE_INSTALL_PREFIX}/include")
MESSAGE(STATUS "/plugins directory: ${CMAKE_INSTALL_PREFIX}/${ISIS_PLUGIN_INFIX}")

# write configuration file
CONFIGURE_FILE(cmake/isis_corecfg.cmake.in ${CMAKE_BINARY_DIR}/ISISConfig.cmake @ONLY)

# install cmake configuration files
INSTALL(FILES ${CMAKE_BINARY_DIR}/ISISConfig.cmake DESTINATION share/isis/cmake COMPONENT Development)

# uninstall target
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/cmake//cmake_uninstall.cmake.in" "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake" IMMEDIATE @ONLY)

add_custom_target(uninstall COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake)
