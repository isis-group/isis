# the ISIS core module
CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

INCLUDE(CPack)
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
SET(ISIS_INSTALL_BIN_DIR ${CMAKE_INSTALL_PREFIX}/bin)
SET(ISIS_INSTALL_LIB_DIR ${CMAKE_INSTALL_PREFIX}/lib)
SET(ISIS_INSTALL_INCLUDE_DIR ${CMAKE_INSTALL_PREFIX}/include/isis)
IF(NOT ISIS_INSTALL_PLUGIN_DIR)
  SET(ISIS_INSTALL_PLUGIN_DIR ${CMAKE_INSTALL_PREFIX}/share/isis/plugins)
ENDIF(NOT ISIS_INSTALL_PLUGIN_DIR)


###########################################################
# CPack stuff
###########################################################
SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY "isis core library")
SET(CPACK_PACKAGE_VENDOR "the friendly isis team")
#SET(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_CURRENT_SOURCE_DIR}/ReadMe.txt")
#SET(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/Copyright.txt")
SET(CPACK_PACKAGE_VERSION_MAJOR ${${CMAKE_PROJECT_NAME}_VERSION_MAJOR})
SET(CPACK_PACKAGE_VERSION_MINOR ${${CMAKE_PROJECT_NAME}_VERSION_MINOR})
SET(CPACK_PACKAGE_VERSION_PATCH ${${CMAKE_PROJECT_NAME}_VERSION_PATCH})
#SET(CPACK_PACKAGE_INSTALL_DIRECTORY "CMake ${${CMAKE_PROJECT_NAME}_VERSION_MAJOR}.${${CMAKE_PROJECT_NAME}_VERSION_MINOR}")

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
ADD_DEFINITIONS(-DPLUGIN_PATH="${ISIS_INSTALL_PLUGIN_DIR}")
ADD_DEFINITIONS(-DDL_SUFFIX="${CMAKE_SHARED_LIBRARY_SUFFIX}")
ADD_DEFINITIONS(-DDL_PREFIX="${CMAKE_SHARED_LIBRARY_PREFIX}")

############################################################
# RCS revision number
############################################################
# Set the revision number if this dir is a svn working copy.
# The revision number will be stored in RCS_REVISION.
FIND_FILE(RCS_DESC_COMMAND git-describe
    PATHS /usr/bin /usr/local/bin
    DOC "The rcs revision description information tool.")

IF (NOT RCS_REVISION)
	IF (RCS_DESC_COMMAND)
		EXECUTE_PROCESS(COMMAND ${RCS_DESC_COMMAND} OUTPUT_VARIABLE OUT WORKING_DIRECTORY ${CMAKE_SOURCE_DIR} OUTPUT_STRIP_TRAILING_WHITESPACE)
		string(REGEX MATCH "[0-9]+-[^-]+$" RCS_REVISION "${OUT}") #@todo wont work with more complex revisions/modified sources
		SET(RCS_REVISION ${RCS_REVISION})
	else(RCS_DESC_COMMAND)
		message(status "No rcs command given")
	ENDIF (RCS_DESC_COMMAND)
ENDIF (NOT RCS_REVISION)

IF (RCS_REVISION)
  MESSAGE(STATUS "RCS Revision: ${RCS_REVISION}")
  ADD_DEFINITIONS(-DISIS_RCS_REVISION=${RCS_REVISION})
ENDIF(RCS_REVISION)

############################################################
# Build config
############################################################
SET(ISIS_BUILD_PROPERTIES LINK_FLAGS "--no-undefined" CLEAN_DIRECT_OUTPUT 1)
SET(ISIS_BUILD_STATIC ON CACHE BOOL "enable build of the static library" )
SET(ISIS_BUILD_DYNAMIC ON CACHE BOOL "enable build of the dynamic library" )

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
	SET_TARGET_PROPERTIES( isis_core PROPERTIES	${ISIS_BUILD_PROPERTIES} VERSION ${${CMAKE_PROJECT_NAME}_VERSION} INSTALL_NAME_DIR ${ISIS_INSTALL_LIB_DIR})
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
  RUNTIME DESTINATION ${ISIS_INSTALL_BIN_DIR} COMPONENT RuntimeLibraries
  LIBRARY DESTINATION ${ISIS_INSTALL_LIB_DIR} COMPONENT RuntimeLibraries
  ARCHIVE DESTINATION ${ISIS_INSTALL_LIB_DIR} COMPONENT Development)

# install header files
INSTALL(FILES ${COREUTILS_HDR_FILES}
  DESTINATION ${ISIS_INSTALL_INCLUDE_DIR}/CoreUtils COMPONENT Development)

INSTALL(FILES ${DATASTORAGE_HDR_FILES}
  DESTINATION ${ISIS_INSTALL_INCLUDE_DIR}/DataStorage COMPONENT Development)

# status messages
MESSAGE(STATUS "/bin directory: " ${ISIS_INSTALL_BIN_DIR}) 
MESSAGE(STATUS "/lib directory: " ${ISIS_INSTALL_LIB_DIR}) 
MESSAGE(STATUS "/include directory: " ${ISIS_INSTALL_INCLUDE_DIR}) 
MESSAGE(STATUS "/plugins directory: " ${ISIS_INSTALL_PLUGIN_DIR})

# write configuration file
CONFIGURE_FILE(cmake/isis_corecfg.cmake.in ${CMAKE_BINARY_DIR}/ISISConfig.cmake @ONLY)

# install cmake configuration files
INSTALL(FILES ${CMAKE_BINARY_DIR}/ISISConfig.cmake
  DESTINATION ${CMAKE_INSTALL_PREFIX}/share/isis/cmake COMPONENT Development)

# uninstall target
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/cmake//cmake_uninstall.cmake.in" "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake" IMMEDIATE @ONLY)

add_custom_target(uninstall COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake)
