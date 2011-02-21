# the isis python module
CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

PROJECT(ISIS_PYTHON)

set(ISIS_PYTHON_INSTALL_DIR "lib/python${CURRENT_PYTHON_VERSION}/site-packages/isis")

###########################################################
# find and configure the isis core library
###########################################################
SET(ISIS_DIR /usr/share/isis/cmake CACHE PATH
    "The directory where the isis cmake configuration skript script is located.")
FIND_PACKAGE(ISIS REQUIRED)
INCLUDE_DIRECTORIES(${ISIS_INCLUDE_DIRS})
MESSAGE(STATUS ${ISIS_INCLUDE_DIRS})


############################################################
# set isis version number
############################################################
SET(${CMAKE_PROJECT_NAME}_VERSION_MAJOR "0")
SET(${CMAKE_PROJECT_NAME}_VERSION_MINOR "1")
SET(${CMAKE_PROJECT_NAME}_VERSION_PATCH "0")

SET(${CMAKE_PROJECT_NAME}_VERSION
  ${${CMAKE_PROJECT_NAME}_VERSION_MAJOR}.${${CMAKE_PROJECT_NAME}_VERSION_MINOR}.${${CMAKE_PROJECT_NAME}_VERSION_PATCH})

# we use the log definitions of the core
IF(ISIS_RUNTIME_LOG)
	ADD_DEFINITIONS(-D_ENABLE_LOG=1)
ELSE(ISIS_RUNTIME_LOG)
	ADD_DEFINITIONS(-D_ENABLE_LOG=0)
ENDIF(ISIS_RUNTIME_LOG)

# enable DEBUG_LOG
IF(ISIS_DEBUG_LOG)
	ADD_DEFINITIONS(-D_ENABLE_DEBUG=1)
	MESSAGE(STATUS "Debug code enabled, this will will have a big impact on the performance")
ELSE(ISIS_DEBUG_LOG)
	ADD_DEFINITIONS(-D_ENABLE_DEBUG=0)
ENDIF(ISIS_DEBUG_LOG)

OPTION(${CMAKE_PROJECT_NAME}_CORE "Build core python library" ON)
OPTION(${CMAKE_PROJECT_NAME}_DATASTORAGE "Build datastorage python library" ON)

FIND_PACKAGE(Boost REQUIRED COMPONENTS python)
FIND_PACKAGE(PythonLibs REQUIRED)
INCLUDE_DIRECTORIES(${PYTHON_INCLUDE_PATH})

add_subdirectory(isis)
configure_file(${PROJECT_SOURCE_DIR}/isis/__init__.py ${PROJECT_BINARY_DIR}/isis/__init__.py COPYONLY)


###########################################################################
# installation
###########################################################################
MESSAGE(STATUS "Installing python modules to ${CMAKE_INSTALL_PREFIX}/${ISIS_PYTHON_INSTALL_DIR}")
install(FILES ${PROJECT_SOURCE_DIR}/isis/__init__.py DESTINATION ${CMAKE_INSTALL_PREFIX}/${ISIS_PYTHON_INSTALL_DIR})
