cmake_minimum_required(VERSION 2.6)

project(ISIS_CALC)

###########################################################
# find and configure the isis core library
###########################################################
SET(ISIS_DIR /usr/share/isis/cmake CACHE PATH
"The directory where the isis cmake configuration script is located.")

FIND_PACKAGE(ISIS REQUIRED)
INCLUDE_DIRECTORIES(${ISIS_INCLUDE_DIRS})

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

FIND_LIBRARY(LIB_MUPARSER "muparser")
add_executable(isiscalc isiscalc.cpp)
target_link_libraries(isiscalc ${LIB_MUPARSER} ${ISIS_LIB} ${ISIS_LIB_DEPENDS})
SET_TARGET_PROPERTIES(isiscalc PROPERTIES INSTALL_RPATH_USE_LINK_PATH TRUE)

# the install targets.
INSTALL(TARGETS isiscalc RUNTIME DESTINATION bin)

MESSAGE(STATUS "Installing binary files to ${CMAKE_INSTALL_PREFIX}/bin")

