# the ISIS core module
CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

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
SET(ISIS_INSTALL_PLUGIN_DIR ${CMAKE_INSTALL_PREFIX}/share/isis/plugins
  CACHE PATH "Path where the plugins shall be installed to.")

############################################################
# set project specific module path
############################################################
SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH}
  "${CMAKE_CURRENT_SOURCE_DIR}/cmake/Modules")

############################################################
# Compiler flags on 64Bit cpus
############################################################
IF("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "x86_64")
  MESSAGE(STATUS "Compiling on x86_64. Using the '-fpic' option")
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
# SVN revision number
############################################################
# Set the revision number if this dir is a svn working copy.
# The revision number will be stored in SVN_REVISION.
FIND_FILE(SVNVERSION svnversion
    PATHS /usr/bin /usr/local/bin
    DOC "The subversion revision information tool.")

IF (NOT SVN_REVISION)
	IF (SVNVERSION)
		EXECUTE_PROCESS(COMMAND ${SVNVERSION}
			OUTPUT_VARIABLE OUT
			WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
		string(REGEX MATCH "exported" EXPORTED "${OUT}")
		IF (NOT EXPORTED)
			string(REGEX MATCH "^[0-9]+" SVN_REVISION "${OUT}") #@todo wont work with more complex revisions/modified sources
			SET(SVN_REVISION ${SVN_REVISION})
		ELSE (NOT EXPORTED)
			set(SVN_REVISION "exported")
		ENDIF (NOT EXPORTED)
	ENDIF (SVNVERSION)
ENDIF (NOT SVN_REVISION)

IF (SVN_REVISION)
  MESSAGE(STATUS "SVN Revision: ${SVN_REVISION}")
  ADD_DEFINITIONS(-D_ISIS_SVN_REVISION=${SVN_REVISION})
ENDIF(SVN_REVISION)

############################################################
# build core
############################################################

# add new source files to this parameter list
SET ( SRC_FILES 
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
  DataStorage/ndimensional.cpp 
  DataStorage/image.cpp 
  DataStorage/io_factory.cpp 
  DataStorage/typeptr_converter.cpp 
  DataStorage/numeric_convert.cpp 
  DataStorage/io_interface.cpp 
  DataStorage/typeptr_base.cpp 
  DataStorage/common.cpp )

SET (COREUTILS_HDR_FILES
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
ADD_LIBRARY( isis SHARED ${SRC_FILES} )
ADD_LIBRARY( isis_static STATIC ${SRC_FILES} )

TARGET_LINK_LIBRARIES( isis ${CMAKE_DL_LIBS} ${Boost_LIBRARIES} )

SET(ISIS_BUILD_PROPERTIES LINK_FLAGS "--no-undefined" CLEAN_DIRECT_OUTPUT 1)

SET_TARGET_PROPERTIES( isis PROPERTIES ${ISIS_BUILD_PROPERTIES} VERSION ${${CMAKE_PROJECT_NAME}_VERSION})
SET_TARGET_PROPERTIES( isis_static PROPERTIES ${ISIS_BUILD_PROPERTIES} OUTPUT_NAME "isis" )

############################################################
# Installation
############################################################

# install libraries
INSTALL (TARGETS isis isis_static
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
CONFIGURE_FILE(cmake/isis_corecfg.cmake.in ISISConfig.cmake @ONLY)

# install cmake configuration files
INSTALL(FILES ${CMAKE_BINARY_DIR}/ISISConfig.cmake
  DESTINATION ${CMAKE_INSTALL_PREFIX}/share/isis/cmake COMPONENT Development)


  
