############################################################
# The ISIS project
# 
# CMake configuration file of the ISIS I/O plugins
#
# Author: Thomas Proeger <thomasproeger@googlemail.com>
# Date: Wed, 06 Apr 2011 19:23:10 +0200
# 
############################################################

############################################################
# include the ISIS core header files
############################################################
include_directories(${CMAKE_SOURCE_DIR}/lib/core)
find_package(ZLIB)

############################################################
# add global options
############################################################
option(ISIS_IOPLUGIN_NIFTI_SA "Enable standalone Nifti-IO plugin" ON)
option(ISIS_IOPLUGIN_NULL "Enable Null-IO plugin" OFF)
option(ISIS_IOPLUGIN_PNG "Enable PNG-IO plugin" OFF)
option(ISIS_IOPLUGIN_DICOM "Enable Dicom-IO plugin" OFF)
option(ISIS_IOPLUGIN_COMP "Enable proxy plugin for compressed data (tar, gz, bzip2 and Z)" ON)
option(ISIS_IOPLUGIN_FLIST "Enable proxy plugin which gets filenames from a file or stdin" ON)
option(ISIS_IOPLUGIN_PROCESS "Enable proxy plugin which gets filenames from a child process" ON)
option(ISIS_IOPLUGIN_SIEMENSTCPIP "Enable plugin for Siemens data coming on TCP Port" OFF)

############################################################
# the plugins ...
############################################################

############################################################
# NIFTI plugin
############################################################
if(ISIS_IOPLUGIN_NIFTI_SA)
	add_library(isisImageFormat_Nifti_sa SHARED imageFormat_nifti_sa.cpp imageFormat_nifti_dcmstack.cpp )
	target_link_libraries(isisImageFormat_Nifti_sa ${isis_core_lib} )
	set(TARGETS ${TARGETS} isisImageFormat_Nifti_sa)
endif(ISIS_IOPLUGIN_NIFTI_SA)


############################################################
# NULL plugin
############################################################
if(ISIS_IOPLUGIN_NULL)
  add_library(isisImageFormat_Null SHARED imageFormat_Null.cpp)
  target_link_libraries(isisImageFormat_Null ${isis_core_lib})
  set(TARGETS ${TARGETS} isisImageFormat_Null)
endif(ISIS_IOPLUGIN_NULL)

############################################################
# PNG plugin
############################################################
if(ISIS_IOPLUGIN_PNG)
	find_package(PNG REQUIRED)
	include_directories(${PNG_INCLUDE_DIRS})

	add_library(isisImageFormat_png SHARED imageFormat_png.cpp)
	target_link_libraries(isisImageFormat_png ${isis_core_lib} ${PNG_LIBRARIES})

	set(TARGETS ${TARGETS} isisImageFormat_png)
endif(ISIS_IOPLUGIN_PNG)

############################################################
# COMP proxy plugin
############################################################
if(ISIS_IOPLUGIN_COMP)
  find_package(Boost REQUIRED COMPONENTS iostreams)

  option(ISIS_IOPLUGIN_COMP_LZMA "Add support for lzma in the plugin for compressed data (xz)" OFF)

  find_library(LIB_BZ2 "bz2")

  list(APPEND COMP_LIBS ${ZLIB_LIBRARIES} ${LIB_BZ2})
  list(APPEND COMP_SRC imageFormat_compressed.cpp)

  if(ISIS_IOPLUGIN_COMP_LZMA)
	find_library(LIB_LZMA "lzma")
	find_path(INCPATH_LZMA "lzma.h")
	include_directories(${INCPATH_LZMA})
	add_definitions("-DHAVE_LZMA")
	list(APPEND COMP_LIBS ${LIB_LZMA})
	list(APPEND COMP_SRC imageFormat_compressed_lzma.cpp)
  endif(ISIS_IOPLUGIN_COMP_LZMA)
  
  add_library(isisImageFormat_comp_proxy SHARED ${COMP_SRC})
  target_link_libraries(isisImageFormat_comp_proxy ${isis_core_lib} ${Boost_IOSTREAMS_LIBRARY} ${COMP_LIBS})
  set(TARGETS ${TARGETS} isisImageFormat_comp_proxy)
endif(ISIS_IOPLUGIN_COMP)

############################################################
# FLIST proxy plugin
############################################################
if(ISIS_IOPLUGIN_FLIST)
	add_library(isisImageFormat_flist SHARED imageFormat_flist.cpp)
	target_link_libraries(isisImageFormat_flist ${isis_core_lib})
	set(TARGETS ${TARGETS} isisImageFormat_flist)
endif(ISIS_IOPLUGIN_FLIST)

############################################################
# PROCESS proxy plugin
############################################################
if(ISIS_IOPLUGIN_PROCESS)
	add_library(isisImageFormat_process SHARED imageFormat_process.cpp)
	target_link_libraries(isisImageFormat_process ${isis_core_lib})
	set(TARGETS ${TARGETS} isisImageFormat_process)
endif(ISIS_IOPLUGIN_PROCESS)

############################################################
# Dicom
############################################################
if(ISIS_IOPLUGIN_DICOM)
	find_package(DCMTK REQUIRED)

	include_directories(${DCMTK_dcmtk_INCLUDE_DIR})

	add_library(isisImageFormat_Dicom SHARED imageFormat_Dicom.cpp imageFormat_DicomParser.cpp)
	target_link_libraries(isisImageFormat_Dicom ${DCMTK_LIBRARIES} ${isis_core_lib})
	set(TARGETS ${TARGETS} isisImageFormat_Dicom)
endif(ISIS_IOPLUGIN_DICOM)

#################################################################################################
# Siemens TCPIP
#################################################################################################

if (ISIS_IOPLUGIN_SIEMENSTCPIP)
	add_library(isisImageFormat_SiemensTcpIp SHARED imageFormat_SiemensTcpIp.cpp)
	message(STATUS)
	target_link_libraries(isisImageFormat_SiemensTcpIp ${isis_core_lib})
	set(TARGETS ${TARGETS} isisImageFormat_SiemensTcpIp)
endif(ISIS_IOPLUGIN_SIEMENSTCPIP)

###########################################################################
# prepare all plugins for installation
###########################################################################
if(WIN32)
message(STATUS "Installing following plugins to ${CMAKE_INSTALL_PREFIX}/bin")
else(WIN32)
message(STATUS "Installing following plugins to ${CMAKE_INSTALL_PREFIX}/${ISIS_PLUGIN_INFIX}")
endif(WIN32)

foreach(PLUGIN ${TARGETS})
  message(STATUS " == ${PLUGIN}")
  set_target_properties(${PLUGIN} PROPERTIES INSTALL_RPATH_USE_LINK_PATH TRUE)
endforeach(PLUGIN)

###########################################################
# actual install
###########################################################
if(WIN32)
install(TARGETS ${TARGETS} RUNTIME DESTINATION bin COMPONENT "IO_plugins")
else(WIN32)
install(TARGETS ${TARGETS} DESTINATION ${ISIS_PLUGIN_INFIX} COMPONENT "IO_plugins" )
endif(WIN32)

# # uninstall target
# configure_file(
# 	"${CMAKE_CURRENT_SOURCE_DIR}/cmake/cmake_uninstall.cmake.in"
# 	"${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
# 	IMMEDIATE @ONLY
# )
