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
include_directories(${CMAKE_SOURCE_DIR}/lib/Core)

############################################################
# add global options
############################################################
option(ISIS_IOPLUGIN_NIFTI "Enable Nifti-IO plugin" OFF)
option(ISIS_IOPLUGIN_NIFTI_SA "Enable standalone Nifti-IO plugin" ON)
option(ISIS_IOPLUGIN_VISTA_SA "Enable standalone Vista-IO plugin" ON)
option(ISIS_IOPLUGIN_NULL "Enable Null-IO plugin" OFF)
option(ISIS_IOPLUGIN_PNG "Enable PNG-IO plugin" OFF)
option(ISIS_IOPLUGIN_DICOM "Enable Dicom-IO plugin" OFF)
option(ISIS_IOPLUGIN_VISTA "Enable Vista-IO plugin" OFF)
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
if(ISIS_IOPLUGIN_NIFTI)
  find_library(LIB_NIFTIIO niftiio)
  find_library(LIB_ZNZ znz)
  find_library(LIB_Z z)

  find_path(INCPATH_NIFTI "nifti1_io.h" PATH_SUFFIXES "nifti")
  find_path(INCPATH_ZNZ "znzlib.h" PATH_SUFFIXES "nifti")

  include_directories(${INCPATH_NIFTI} ${INCPATH_ZNZ})
  add_library(isisImageFormat_Nifti SHARED imageFormat_Nifti.cpp)
  target_link_libraries(isisImageFormat_Nifti ${LIB_NIFTIIO} ${LIB_ZNZ} ${LIB_Z} isis_core )

  set(TARGETS ${TARGETS} isisImageFormat_Nifti)
endif(ISIS_IOPLUGIN_NIFTI)

if(ISIS_IOPLUGIN_NIFTI_SA)
  add_library(isisImageFormat_Nifti_sa SHARED imageFormat_nifti_sa.cpp)
  target_link_libraries(isisImageFormat_Nifti_sa isis_core )
  set(TARGETS ${TARGETS} isisImageFormat_Nifti_sa)
endif(ISIS_IOPLUGIN_NIFTI_SA)

############################################################
# vista
############################################################

if(ISIS_IOPLUGIN_VISTA_SA)
	add_library(isisImageFormat_Vista_sa SHARED Vista_sa/VistaSaParser.cpp Vista_sa/imageFormat_Vista_sa.cpp)
	target_link_libraries(isisImageFormat_Vista_sa isis_core )
	set(TARGETS ${TARGETS} isisImageFormat_Vista_sa)
endif(ISIS_IOPLUGIN_VISTA_SA)

if(ISIS_IOPLUGIN_VISTA)
  # This code snipped was taken from the lipsia source package.

  # - FindVIA.cmake
  #
  # Author: Thomas Proeger
  #
  # Find the header files and libraries for the libvia and libvia-dev packages
  #
  # This find package exports the following variables:
  # VIA_INCLUDE_DIR         - directory for via.h viadata.h
  # VIAIO_INCLUDE_DIR       - directory for VImage.h, option.h, etc. pp
  # VIA_LIBRARY             - libvia
  # VIAIO_LIBRARY           - libviaio
  # VX_LIBRARY              - libvx
  #

  # SET a root VIA include directory if necessary. This should be the directory
  # containing the via and viaio include dirs.
  if(VIA_ROOT_INCLUDE_DIR)
    include_directories(${VIA_ROOT_INCLUDE_DIR})
  endif(VIA_ROOT_INCLUDE_DIR)

  # include directories
  find_path(VIA_INCLUDE_DIR
    NAMES via.h
    PATH_SUFFIXES via
    DOC "The include directory containing via.h"
    )

  find_path(VIAIO_INCLUDE_DIR
    NAMES VImage.h
    PATH_SUFFIXES viaio
    DOC "The include directory containing VImage.h and option.h"
    )

  # LIBRARY files
  find_library(VIA_LIBRARY
    NAMES via libvia
    DOC "The library file libvia.so"
    )

  find_library(VIAIO_LIBRARY
    NAMES viaio libviaio
    DOC "The library file libviaio.so"
    )

  find_library(VX_LIBRARY
    NAMES vx libvx
    DOC "The library file libvx.so"
    )

  # these variables are only visible in 'advanced mode'
  mark_as_advanced(VIA_INCLUDE_DIR
    VIAIO_INCLUDE_DIR
    VIA_LIBRARY
    VIAIO_LIBRARY
    VX_LIBRARY
    )

  # if via components found configure build environment
  if(VIA_INCLUDE_DIR AND VIAIO_INCLUDE_DIR
      AND VIA_LIBRARY AND VIAIO_LIBRARY)

    include_directories(${VIA_INCLUDE_DIR} ${VIAIO_INCLUDE_DIR})
    add_library(isisImageFormat_Vista SHARED imageFormat_Vista.cpp)
    target_link_libraries(isisImageFormat_Vista ${VIAIO_LIBRARY} isis_core)
    set(TARGETS ${TARGETS} isisImageFormat_Vista)
  else(VIA_INCLUDE_DIR AND VIAIO_INCLUDE_DIR
      AND VIA_LIBRARY AND VIAIO_LIBRARY)

    message(FATAL_ERROR, "Cannot find viaiio libraries. Please install the package libvia-dev")

  endif(VIA_INCLUDE_DIR AND VIAIO_INCLUDE_DIR
    AND VIA_LIBRARY AND VIAIO_LIBRARY)
endif(ISIS_IOPLUGIN_VISTA)

############################################################
# NULL plugin
############################################################
if(ISIS_IOPLUGIN_NULL)
  add_library(isisImageFormat_Null SHARED imageFormat_Null.cpp)
  target_link_libraries(isisImageFormat_Null isis_core)
  set(TARGETS ${TARGETS} isisImageFormat_Null)
endif(ISIS_IOPLUGIN_NULL)

############################################################
# PNG plugin
############################################################
if(ISIS_IOPLUGIN_PNG)
  find_library(LIB_Z z)
  find_library(LIB_PNG png12)

  find_path(INCPATH_PNG "png.h")
  include_directories(${INCPATH_PNG})

  add_library(isisImageFormat_png SHARED imageFormat_png.cpp)
  target_link_libraries(isisImageFormat_png isis_core ${LIB_PNG} ${LIB_Z})

  set(TARGETS ${TARGETS} isisImageFormat_png)
endif(ISIS_IOPLUGIN_PNG)

############################################################
# COMP proxy plugin
############################################################
if(ISIS_IOPLUGIN_COMP)
  find_package(Boost REQUIRED COMPONENTS iostreams)
  add_library(isisImageFormat_comp_proxy SHARED imageFormat_compressed.cpp)
  
  find_library(LIB_Z "z")
  find_library(LIB_BZ2 "bz2")

  target_link_libraries(isisImageFormat_comp_proxy isis_core ${Boost_IOSTREAMS_LIBRARY} ${LIB_Z} ${LIB_BZ2})
  set(TARGETS ${TARGETS} isisImageFormat_comp_proxy)
endif(ISIS_IOPLUGIN_COMP)

############################################################
# FLIST proxy plugin
############################################################
if(ISIS_IOPLUGIN_FLIST)
	add_library(isisImageFormat_flist SHARED imageFormat_flist.cpp)
	target_link_libraries(isisImageFormat_flist isis_core)
	set(TARGETS ${TARGETS} isisImageFormat_flist)
endif(ISIS_IOPLUGIN_FLIST)

############################################################
# PROCESS proxy plugin
############################################################
if(ISIS_IOPLUGIN_PROCESS)
	add_library(isisImageFormat_process SHARED imageFormat_process.cpp)
	target_link_libraries(isisImageFormat_process isis_core)
	set(TARGETS ${TARGETS} isisImageFormat_process)
endif(ISIS_IOPLUGIN_PROCESS)

############################################################
# Dicom
############################################################
if(ISIS_IOPLUGIN_DICOM)
	find_library(LIB_DCMIMGLE dcmimgle)
	find_library(LIB_DCMIMAGE dcmimage)
	find_library(LIB_DCMDATA dcmdata)
	find_library(LIB_OFSTD ofstd)

	find_library(LIB_Z z)
	find_library(LIB_TIFF tiff)
	find_library(LIB_PNG png12)

	set(DCM_LIBS ${LIB_DCMIMGLE} ${LIB_DCMIMAGE}  ${LIB_DCMDATA}) #higher functions

	find_path(INCPATH_DCMTK "dcmtk/dcmdata/dcfilefo.h")
	include_directories(${INCPATH_DCMTK})
	
	find_library(LIB_DCM_LOG "oflog")
	if(LIB_DCM_LOG)
		message(STATUS "dcmtk 3.6 found - linking against ${LIB_DCM_LOG}")
		list(APPEND DCM_LIBS ${LIB_DCM_LOG} )
	endif(LIB_DCM_LOG)

	list(APPEND DCM_LIBS ${LIB_OFSTD}  ${LIB_Z} ${LIB_TIFF} ${LIB_PNG}) #low level functions

	add_library(isisImageFormat_Dicom SHARED imageFormat_Dicom.cpp imageFormat_DicomParser.cpp)

	if(WIN32)
	list(APPEND DCM_LIBS ws2_32 netapi32 wsock32)
	endif(WIN32)

	target_link_libraries(isisImageFormat_Dicom ${DCM_LIBS} isis_core)
	set(TARGETS ${TARGETS} isisImageFormat_Dicom)
endif(ISIS_IOPLUGIN_DICOM)

#################################################################################################
# Siemens TCPIP
#################################################################################################

if (ISIS_IOPLUGIN_SIEMENSTCPIP)
	add_library(isisImageFormat_SiemensTcpIp SHARED imageFormat_SiemensTcpIp.cpp)
	message(STATUS)
	target_link_libraries(isisImageFormat_SiemensTcpIp isis_core)
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
  set_target_properties(${PLUGIN} PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR} INSTALL_RPATH_USE_LINK_PATH TRUE)
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
