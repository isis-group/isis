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

  qt4_wrap_cpp(qdefaultmessageprint_moc qt4/qdefaultmessageprint.hpp)
  # configure target
  add_library(isisAdapter_qt4 SHARED qt4/qtapplication.cpp ${qdefaultmessageprint_moc} qt4/qdefaultmessageprint.cpp )
  target_link_libraries(isisAdapter_qt4 isis_core ${ISIS_LIB_DEPENDS} ${QT_LIBRARIES} ${CMAKE_C_STANDARD_LIBRARIES})

  # configure install destination
  install (TARGETS isisAdapter_qt4
	RUNTIME DESTINATION bin COMPONENT RuntimeLibraries
	LIBRARY DESTINATION lib COMPONENT RuntimeLibraries
	ARCHIVE DESTINATION lib COMPONENT Development
  )

  # export header files
  set(ADAPTER_HDR_FILES ${ADAPTER_HDR_FILES} qt4/qtapplication.hpp qt4/qdefaultmessageprint.hpp )  
endif(ISIS_QT4)

############################################################
# Install header and helper files
############################################################
install(FILES 
  ${ADAPTER_HDR_FILES} DESTINATION ${CMAKE_INSTALL_PREFIX}/include/isis/Adapter COMPONENT Development)
