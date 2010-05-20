cmake_minimum_required(VERSION 2.6)

# the ISIS project
project(ISIS)

#set isis version number
SET(${CMAKE_PROJECT_NAME}_VERSION_MAJOR "0")
SET(${CMAKE_PROJECT_NAME}_VERSION_MINOR "1")
SET(${CMAKE_PROJECT_NAME}_VERSION_PATCH "0")

add_definitions(-D_ISIS_VERSION_MAJOR=${${CMAKE_PROJECT_NAME}_VERSION_MAJOR})
add_definitions(-D_ISIS_VERSION_MINOR=${${CMAKE_PROJECT_NAME}_VERSION_MINOR})
add_definitions(-D_ISIS_VERSION_PATCH=${${CMAKE_PROJECT_NAME}_VERSION_PATCH})


###############################################################################
#Global options
###############################################################################

# set project specific module path
SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH}
  "${CMAKE_CURRENT_SOURCE_DIR}/cmake/Modules")

FIND_PACKAGE(Boost REQUIRED COMPONENTS filesystem regex system date_time)
include_directories(${Boost_INCLUDE_DIR}  lib)

OPTION(${CMAKE_PROJECT_NAME}_BUILD_APPLICATIONS "Build the isis application directory" OFF)

OPTION(${CMAKE_PROJECT_NAME}_BUILD_TESTS "Build the tests" ON)
OPTION(BUILD_SHARED_LIBS "Build isis with shared libraries" ON)

OPTION(${CMAKE_PROJECT_NAME}_BUILD_COREUTILS "Build the isis coreutils" ON)
OPTION(${CMAKE_PROJECT_NAME}_BUILD_DATA_IO "Build the isis data and io modules" ON)

OPTION(${CMAKE_PROJECT_NAME}_HAVE_ITK "Enable ITK-support" OFF)
OPTION(${CMAKE_PROJECT_NAME}_HAVE_VTK "Enable VTK-support" OFF)
OPTION(${CMAKE_PROJECT_NAME}_HAVE_QT4 "Enable Qt4-support" OFF)

OPTION(${CMAKE_PROJECT_NAME}_RUNTIME_LOG "Enable runtime logging" ON)
OPTION(${CMAKE_PROJECT_NAME}_DEBUG_LOG "Enable debug logging" ON)

OPTION(${CMAKE_PROJECT_NAME}_BUILD_DOC "Build the Doxygen documentation for the core libraries" OFF)

################################################################################
# ISIS project version number
################################################################################)

SET(ISIS_VERSION
  ${${CMAKE_PROJECT_NAME}_VERSION_MAJOR}.${${CMAKE_PROJECT_NAME}_VERSION_MINOR}.${${CMAKE_PROJECT_NAME}_VERSION_PATCH})

###############################################################################
#Global definitions/compiler flags
###############################################################################
if(${CMAKE_PROJECT_NAME}_RUNTIME_LOG)
	add_definitions(-D_ENABLE_LOG=1)
else(${CMAKE_PROJECT_NAME}_RUNTIME_LOG)
	add_definitions(-D_ENABLE_LOG=0)
endif(${CMAKE_PROJECT_NAME}_RUNTIME_LOG)

if(${CMAKE_PROJECT_NAME}_DEBUG_LOG)
	add_definitions(-D_ENABLE_DEBUG=1)
	MESSAGE(STATUS "Debug code enabled, this will will have a big impact on the performance")
else(${CMAKE_PROJECT_NAME}_DEBUG_LOG)
	add_definitions(-D_ENABLE_DEBUG=0)
endif(${CMAKE_PROJECT_NAME}_DEBUG_LOG)

#If we do a static build at least the plugins will be dynamic, so we need -fPIC
if (NOT BUILD_SHARED_LIBS)
	IF("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "x86_64")
		SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC" )
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
			SET(SVN_REVISION r${SVN_REVISION})
		ELSE (NOT EXPORTED)
			set(SVN_REVISION "exported")
		ENDIF (NOT EXPORTED)
	ENDIF (SVNVERSION)
ENDIF (NOT SVN_REVISION)

IF (SVN_REVISION)
  message(STATUS "SVN Revision: ${SVN_REVISION}")
  add_definitions(-D_ISIS_SVN_REVISION=${SVN_REVISION})
endif(SVN_REVISION)

IF(${CMAKE_PROJECT_NAME}_HAVE_ITK)
	FIND_PACKAGE(ITK REQUIRED)
	include(${USE_ITK_FILE})
ENDIF(${CMAKE_PROJECT_NAME}_HAVE_ITK)

IF(${CMAKE_PROJECT_NAME}_HAVE_VTK)
	FIND_PACKAGE(VTK REQUIRED)
	include(${USE_VTK_FILE})
ENDIF(${CMAKE_PROJECT_NAME}_HAVE_VTK)

IF(${CMAKE_PROJECT_NAME}_HAVE_QT4)
	add_definitions(-D_HAVE_QT4=${CMAKE_PROJECT_NAME}_HAVE_QT4)
	find_package(Qt4 REQUIRED)
	include(${QT_USE_FILE})
ENDIF(${CMAKE_PROJECT_NAME}_HAVE_QT4)

# If ITK AND VTK are used together we have to increase the number of maximal
# template recursion otherwise there will be an overflow in the Boost library.
IF(ITK_FOUND OR VTK_FOUND)
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ftemplate-depth-60")
ENDIF(ITK_FOUND OR VTK_FOUND)

# configure documentation
IF(${CMAKE_PROJECT_NAME}_BUILD_DOC)
  INCLUDE(UseDoxygen)
ENDIF(${CMAKE_PROJECT_NAME}_BUILD_DOC)

###############################################################################
#Subdirs
###############################################################################

add_subdirectory(lib)
add_subdirectory(tools)
add_subdirectory(design)

IF(${CMAKE_PROJECT_NAME}_BUILD_APPLICATIONS)
	add_subdirectory(apps)
ENDIF(${CMAKE_PROJECT_NAME}_BUILD_APPLICATIONS)

IF(${CMAKE_PROJECT_NAME}_BUILD_TESTS)
    add_subdirectory(tests)
ENDIF(${CMAKE_PROJECT_NAME}_BUILD_TESTS)
