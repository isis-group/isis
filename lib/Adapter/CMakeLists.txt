# ths ISIS adapter module
CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

PROJECT(ISIS_ADAPTER)

###########################################################
# find and configure the isis core library
###########################################################
SET(ISIS_DIR /usr/share/isis/cmake CACHE PATH 
    "The directory where the isis cmake configuration skript script is located.")
FIND_PACKAGE(ISIS REQUIRED)
INCLUDE_DIRECTORIES(${ISIS_INCLUDE_DIRS})

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

OPTION(${CMAKE_PROJECT_NAME}_ITK_ADAPTER "Enable adapter to the ITK library" OFF)
OPTION(${CMAKE_PROJECT_NAME}_VTK_ADAPTER "Enable adapter to the VTK library" OFF)
OPTION(${CMAKE_PROJECT_NAME}_QT4_ADAPTER "Enable adapter to the Qt4 library" OFF)

if(${CMAKE_PROJECT_NAME}_ITK_ADAPTER)
	FIND_PACKAGE(ITK REQUIRED)
	INCLUDE(${USE_ITK_FILE})
	SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ftemplate-depth-100")
# 	add_library(isisAdapter_itk itkAdapter.cpp)
	add_library(isisAdapter_itk SHARED itkAdapter.hpp)
	set_target_properties(isisAdapter_itk PROPERTIES LINKER_LANGUAGE C)
	set_target_properties(isisAdapter_itk PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
	  INSTALL_RPATH_USE_LINK_PATH TRUE)
	target_link_libraries(isisAdapter_itk ${ISIS_LIB} )
	SET(TARGETS ${TARGETS} isisAdapter_itk)
endif(${CMAKE_PROJECT_NAME}_ITK_ADAPTER)

if(${CMAKE_PROJECT_NAME}_VTK_ADAPTER)
	FIND_PACKAGE(VTK REQUIRED)
	INCLUDE(${USE_VTK_FILE})
# 	add_library(isisAdapter_vtk vtkAdapter.cpp)
	add_library(isisAdapter_vtk SHARED vtkAdapter.cpp)
	target_link_libraries(isisAdapter_vtk ${ISIS_LIB} )
	set_target_properties(isisAdapter_vtk PROPERTIES LINK_FLAGS "--no-undefined" )
	set_target_properties(isisAdapter_vtk PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
	  INSTALL_RPATH_USE_LINK_PATH TRUE)
	SET(TARGETS ${TARGETS} isisAdapter_vtk)
endif(${CMAKE_PROJECT_NAME}_VTK_ADAPTER)


if(${CMAKE_PROJECT_NAME}_Qt4_ADAPTER)
	add_subdirectory(qt4)
endif(${CMAKE_PROJECT_NAME}_Qt4_ADAPTER)

MESSAGE(STATUS "Installing plugins to ${CMAKE_INSTALL_PREFIX}/lib")

INSTALL(TARGETS ${TARGETS}
  LIBRARY DESTINATION  ${CMAKE_INSTALL_PREFIX}/lib COMPONENT RuntimeLibraries)
