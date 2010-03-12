cmake_minimum_required(VERSION 2.6)

# the ISIS project
project(isis)

###############################################################################
#Global options
###############################################################################
FIND_PACKAGE(Boost REQUIRED COMPONENTS filesystem regex system date_time)
include_directories(${Boost_INCLUDE_DIR}  lib)

OPTION(BUILD_APPLICATIONS "Build the isis application directory" OFF)

OPTION(BUILD_TESTS "Build the tests" OFF)
OPTION(BUILD_SHARED_LIBS "Build isis with shared libraries" ON)

OPTION(BUILD_COREUTILS "Build the isis coreutils" ON)
OPTION(BUILD_DATA_IO "Build the isis data and io modules" ON)

OPTION(HAVE_ITK "Enable ITK-support" OFF)

OPTION(RUNTIME_LOG "Enable runtime logging" ON)
OPTION(DEBUG_LOG "Enable debug logging" ON)

OPTION(IOPLUGIN_NIFTI "Enable Nifti-IO plugin" OFF)
OPTION(IOPLUGIN_NULL "Enable Null-IO plugin" ON)
OPTION(IOPLUGIN_DICOM "Enable Dicom-IO plugin" OFF)
OPTION(IOPLUGIN_GZ "Enable proxy plugin for compressed files" ON)

###############################################################################
#Global definitions/compiler flags
###############################################################################
if(RUNTIME_LOG)
	add_definitions(-D_ENABLE_DATA_LOG=1)
	add_definitions(-D_ENABLE_CORE_LOG=1)
	add_definitions(-D_ENABLE_IMAGEIO_LOG=1)
else(RUNTIME_LOG)
	add_definitions(-D_ENABLE_DATA_LOG=0)
	add_definitions(-D_ENABLE_CORE_LOG=0)
	add_definitions(-D_ENABLE_IMAGEIO_LOG=0)
endif(RUNTIME_LOG)

if(DEBUG_LOG)
	add_definitions(-D_ENABLE_DATA_DEBUG=1)
	add_definitions(-D_ENABLE_CORE_DEBUG=1)
	add_definitions(-D_ENABLE_IMAGEIO_DEBUG=1)
else(DEBUG_LOG)
	add_definitions(-D_ENABLE_DATA_DEBUG=0)
	add_definitions(-D_ENABLE_CORE_DEBUG=0)
	add_definitions(-D_ENABLE_IMAGEIO_DEBUG=0)
endif(DEBUG_LOG)

#If we do a static build at least the plugins will be dynamic, so we need -fPIC
if (NOT BUILD_SHARED_LIBS)
	IF("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "x86_64")
		SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC"  )
		SET(CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS} -fPIC"  )
		message(STATUS "Using -fPIC also for static build, because we'll need it for the plugins on ${CMAKE_SYSTEM_PROCESSOR}")
	elseif("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "x86_64")
		message(STATUS "Assuming -fPIC will not be necessary for ${CMAKE_SYSTEM_PROCESSOR}")
	ENDIF("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "x86_64")
endif(NOT BUILD_SHARED_LIBS)
	
add_definitions(-DSOURCE_ROOT="${CMAKE_HOME_DIRECTORY}")

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
		ELSE (NOT EXPORTED)
			set(SVN_REVISION "exported")
		ENDIF (NOT EXPORTED)
	ENDIF (SVNVERSION)
ENDIF (NOT SVN_REVISION)

IF (SVN_REVISION)
  message(STATUS "SVN Revision: ${SVN_REVISION}")
  add_definitions(-D_SVN_REVISION=${SVN_REVISION})
endif(SVN_REVISION)

IF(HAVE_ITK)
	FIND_PACKAGE(ITK REQUIRED)
ENDIF(HAVE_ITK)

###############################################################################
#Subdirs
###############################################################################

add_subdirectory(lib)
add_subdirectory(tools)

IF(BUILD_APPLICATIONS)
	add_subdirectory(apps)
ENDIF(BUILD_APPLICATIONS)

IF(BUILD_TESTS)
    add_subdirectory(tests)
ENDIF(BUILD_TESTS)
