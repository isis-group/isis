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
MESSAGE(STATUS ${ISIS_INCLUDE_DIRS})

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

OPTION(${CMAKE_PROJECT_NAME}_ITK "Enable adapter to the ITK library" OFF)
OPTION(${CMAKE_PROJECT_NAME}_VTK "Enable adapter to the VTK library" OFF)
OPTION(${CMAKE_PROJECT_NAME}_QT4 "Enable adapter to the Qt4 library" OFF)

if(${CMAKE_PROJECT_NAME}_ITK)
    add_subdirectory(itk)
    SET(ADAPTER_HDR_FILES ${ADAPTER_HDR_FILES} itk/itkAdapter.hpp itk/itkAdapter_impl.hpp)
    SET(HELPER_HDR_FILES ${HELPER_HDR_FILES} itk/helpFiles/isisTimeStepExtractionFilter.hpp itk/helpFiles/isisTimeStepExtractionFilter_impl.hpp )
    SET(HELPER_HDR_FILES ${HELPER_HDR_FILES} itk/helpFiles/isisIterationObserver.hpp)
endif(${CMAKE_PROJECT_NAME}_ITK)

if(${CMAKE_PROJECT_NAME}_VTK)
	add_subdirectory(vtk)
	SET(ADAPTER_HDR_FILES ${ADAPTER_HDR_FILES} vtk/vtkAdapter.hpp)
endif(${CMAKE_PROJECT_NAME}_VTK)

if(${CMAKE_PROJECT_NAME}_QT4)
	add_subdirectory(qt4)
	SET(ADAPTER_HDR_FILES ${ADAPTER_HDR_FILES} qt4/qtapplication.hpp)
endif(${CMAKE_PROJECT_NAME}_QT4)

# install adapter header files
INSTALL(FILES ${ADAPTER_HDR_FILES} DESTINATION ${CMAKE_INSTALL_PREFIX}/include/isis/Adapter COMPONENT Development)

# install helper header files
INSTALL(FILES ${HELPER_HDR_FILES} DESTINATION ${CMAKE_INSTALL_PREFIX}/include/isis/extITK COMPONENT Development)
  
MESSAGE(STATUS "Targets: ${TARGETS}")

INSTALL(TARGETS ${TARGETS} LIBRARY DESTINATION  ${CMAKE_INSTALL_PREFIX}/lib COMPONENT RuntimeLibraries)
