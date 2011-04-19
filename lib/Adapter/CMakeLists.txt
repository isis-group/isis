############################################################
# The ISIS project
# 
# CMake configuration file for the lib subdir
#
# Author: Thomas Proeger <thomasproeger@googlemail.com>
# Date: Fri, 08 Apr 2011 13:11:34 +0200
# 
############################################################

############################################################
# include core header files
############################################################
include_directories(${CMAKE_SOURCE_DIR}/lib/Core)

############################################################
# optional components
############################################################
option(ISIS_ITK "Enable adapter to the ITK library" OFF)
# remove option until build issues are solved.
# option(ISIS_VTK "Enable adapter to the VTK library" OFF)
option(ISIS_QT4 "Enable adapter to the Qt4 library" OFF)

############################################################
# use ITK component
############################################################
if(ISIS_ITK)
  # export header files
  set(ADAPTER_HDR_FILES 
    itk/itkAdapter.hpp itk/itkAdapter_impl.hpp)
endif(ISIS_ITK)

############################################################
# use QT4 component
############################################################
if(ISIS_QT4)
  # find qt4
  find_package(Qt4 REQUIRED)
  include(${QT_USE_FILE})
  
  # configure target
  add_library(isisAdapter_qt4 SHARED qt4/qtapplication.cpp)
  target_link_libraries(isisAdapter_qt4 isis_core ${QT_LIBRARIES})

  # configure install destination
  install(TARGETS isisAdapter_qt4
    LIBRARY DESTINATION  ${CMAKE_INSTALL_PREFIX}/lib COMPONENT RuntimeLibraries)

  # export header files
  set(ADAPTER_HDR_FILES ${ADAPTER_HDR_FILES} qt4/qtapplication.hpp)  
endif(ISIS_QT4)

if(ISIS_VTK)
  # finde vtk
  find_package(VTK REQUIRED)
  include(${USE_VTK_FILE})
  
  # configure target
  add_library(isisAdapter_vtk SHARED vtk/vtkAdapter.cpp)
  target_link_libraries(isisAdapter_vtk isis_core)
  set_target_properties(isisAdapter_vtk PROPERTIES 
    LINK_FLAGS "--no-undefined"
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR} 
    INSTALL_RPATH_USE_LINK_PATH TRUE)

  # configure install destination
  install(TARGETS isisAdapter_vtk
    LIBRARY DESTINATION ${CMAKE_INSTALL_PREFIX}/lib COMPONENT RuntimeLibraries)

  # export header files
  set(ADAPTER_HDR_FILES ${ADAPTER_HDR_FILES} vtk/vtkAdapter.hpp)  
endif(ISIS_VTK)

############################################################
# Install header and helper files
############################################################
install(FILES 
  ${ADAPTER_HDR_FILES} DESTINATION ${CMAKE_INSTALL_PREFIX}/include/isis/Adapter COMPONENT Development)
