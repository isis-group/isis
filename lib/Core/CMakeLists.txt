# the ISIS core module
CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

PROJECT(ISIS_CORE)

############################################################
# set isis version number
############################################################
SET(${CMAKE_PROJECT_NAME}_VERSION_MAJOR "0")
SET(${CMAKE_PROJECT_NAME}_VERSION_MINOR "1")
SET(${CMAKE_PROJECT_NAME}_VERSION_PATCH "0")

ADD_DEFINITIONS(-D_ISIS_VERSION_MAJOR=${${CMAKE_PROJECT_NAME}_VERSION_MAJOR})
ADD_DEFINITIONS(-D_ISIS_VERSION_MINOR=${${CMAKE_PROJECT_NAME}_VERSION_MINOR})
ADD_DEFINITIONS(-D_ISIS_VERSION_PATCH=${${CMAKE_PROJECT_NAME}_VERSION_PATCH})

SET(ISIS_VERSION
  ${${CMAKE_PROJECT_NAME}_VERSION_MAJOR}.${${CMAKE_PROJECT_NAME}_VERSION_MINOR}.${${CMAKE_PROJECT_NAME}_VERSION_PATCH})

############################################################
# set project specific module path
############################################################
SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH}
  "${CMAKE_CURRENT_SOURCE_DIR}/cmake/Modules")

############################################################
# build static or dynamic libraries
############################################################
OPTION(BUILD_SHARED_LIBS "Build isis with shared libraries" ON)
IF (NOT BUILD_SHARED_LIBS)
  #If we do a static build at least the plugins will be dynamic, so we need -fPIC
  IF("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "x86_64")
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC" )
    SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC"  )
    MESSAGE(STATUS "Using -fPIC also for static build, because we'll need it for the plugins ON ${CMAKE_SYSTEM_PROCESSOR}")
  ELSE("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "x86_64")
    MESSAGE(STATUS "Assuming -fPIC will NOT be necessary for ${CMAKE_SYSTEM_PROCESSOR}")
  ENDIF("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "x86_64")
  # build boost with static libs
  SET(Boost_USE_STATIC_LIBS ON)
ENDIF(NOT BUILD_SHARED_LIBS)

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

# set the plugin directory and the system dependend shared library naming conventions
SET(ISIS_PLUGIN_PATH ${PROJECT_BINARY_DIR} CACHE PATH "Path where the plugins shall be installed.")
ADD_DEFINITIONS(-DPLUGIN_PATH="${ISIS_PLUGIN_PATH}")
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

# the core library
ADD_LIBRARY( isis 
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

TARGET_LINK_LIBRARIES( isis ${CMAKE_DL_LIBS} ${Boost_LIBRARIES} )

SET_TARGET_PROPERTIES( isis PROPERTIES LINK_FLAGS "--no-undefined" )
