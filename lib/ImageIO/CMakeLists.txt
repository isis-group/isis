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

option(${CMAKE_PROJECT_NAME}_IOPLUGIN_NIFTI "Enable Nifti-IO plugin" OFF)
option(${CMAKE_PROJECT_NAME}_IOPLUGIN_NULL "Enable Null-IO plugin" ON)
option(${CMAKE_PROJECT_NAME}_IOPLUGIN_PNG "Enable Null-IO plugin" OFF)
option(${CMAKE_PROJECT_NAME}_IOPLUGIN_DICOM "Enable Dicom-IO plugin" OFF)
option(${CMAKE_PROJECT_NAME}_IOPLUGIN_VISTA "Enable Vista-IO plugin" OFF)
option(${CMAKE_PROJECT_NAME}_IOPLUGIN_GZ "Enable proxy plugin for compressed files" ON)
option(${CMAKE_PROJECT_NAME}_IOPLUGIN_TAR "Enable proxy plugin for tar datasets" ON)
option(${CMAKE_PROJECT_NAME}_IOPLUGIN_RAW "Enable plugin for raw data output" ON)

############################################################
# the plugins ...
############################################################

############################################################
# NIFTI plugin
############################################################
if(${CMAKE_PROJECT_NAME}_IOPLUGIN_NIFTI)
  find_library(LIB_NIFTIIO niftiio)
  find_library(LIB_ZNZ znz)
  find_library(LIB_Z z)

  find_path(INCPATH_NIFTI "nifti1_io.h" PATH_SUFFIXES "nifti")
  find_path(INCPATH_ZNZ "znzlib.h" PATH_SUFFIXES "nifti")

  include_directories(${INCPATH_NIFTI} ${INCPATH_ZNZ})
  add_library(isisImageFormat_Nifti SHARED imageFormat_Nifti.cpp)
  target_link_libraries(isisImageFormat_Nifti ${LIB_NIFTIIO} ${LIB_ZNZ} ${LIB_Z} isis_core  ${ISIS_LIB_DEPENDS})

  set(TARGETS ${TARGETS} isisImageFormat_Nifti)
endif(${CMAKE_PROJECT_NAME}_IOPLUGIN_NIFTI)

############################################################
# vista
############################################################
if(${CMAKE_PROJECT_NAME}_IOPLUGIN_VISTA)
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
    target_link_libraries(isisImageFormat_Vista ${VIAIO_LIBRARY} isis_core ${ISIS_LIB_DEPENDS})
    set(TARGETS ${TARGETS} isisImageFormat_Vista)
  else(VIA_INCLUDE_DIR AND VIAIO_INCLUDE_DIR
      AND VIA_LIBRARY AND VIAIO_LIBRARY)

    message(FATAL_ERROR, "Cannot find viaiio libraries. Please install the package libvia-dev")

  endif(VIA_INCLUDE_DIR AND VIAIO_INCLUDE_DIR
    AND VIA_LIBRARY AND VIAIO_LIBRARY)
endif(${CMAKE_PROJECT_NAME}_IOPLUGIN_VISTA)

############################################################
# NULL plugin
############################################################
if(${CMAKE_PROJECT_NAME}_IOPLUGIN_NULL)
  add_library(isisImageFormat_Null SHARED imageFormat_Null.cpp)
  target_link_libraries(isisImageFormat_Null isis_core ${ISIS_LIB_DEPENDS})
  set(TARGETS ${TARGETS} isisImageFormat_Null)
endif(${CMAKE_PROJECT_NAME}_IOPLUGIN_NULL)

############################################################
# PNG plugin
############################################################
if(${CMAKE_PROJECT_NAME}_IOPLUGIN_PNG)
  find_library(LIB_Z z)
  find_library(LIB_PNG png12)

  find_path(INCPATH_PNG "png.h")
  include_directories(${INCPATH_PNG})

  add_library(isisImageFormat_png SHARED imageFormat_png.cpp)
  target_link_libraries(isisImageFormat_png ${LIB_Z} isis_core ${ISIS_LIB_DEPENDS} ${LIB_PNG})

  set(TARGETS ${TARGETS} isisImageFormat_png)
endif(${CMAKE_PROJECT_NAME}_IOPLUGIN_PNG)

############################################################
# GZ proxy plugin
############################################################
if(${CMAKE_PROJECT_NAME}_IOPLUGIN_GZ)
  find_library(LIB_Z "z")
  find_path(INCPATH_GZIP "zlib.h")
  include_directories(${INCPATH_GZIP})

  add_library(isisImageFormat_gz_proxy SHARED imageFormat_gz_proxy.cpp)
  target_link_libraries(isisImageFormat_gz_proxy ${LIB_Z} isis_core ${ISIS_LIB_DEPENDS})

  set(TARGETS ${TARGETS} isisImageFormat_gz_proxy)
endif(${CMAKE_PROJECT_NAME}_IOPLUGIN_GZ)

############################################################
# TAR proxy plugin
############################################################
if(${CMAKE_PROJECT_NAME}_IOPLUGIN_TAR)
  find_package(Boost REQUIRED COMPONENTS iostreams)

  add_library(isisImageFormat_tar_proxy SHARED imageFormat_tar_proxy.cpp)

  find_library(LIB_Z "z")
  find_library(LIB_BZ2 "bz2")

  if(HAVE_FALLOCATE)
    set_target_properties(isisImageFormat_tar_proxy PROPERTIES COMPILE_DEFINITIONS HAVE_FALLOCATE)
  else(HAVE_FALLOCATE)
    if(HAVE_POSIX_FALLOCATE)
      set_target_properties(isisImageFormat_tar_proxy PROPERTIES COMPILE_DEFINITIONS HAVE_POSIX_FALLOCATE)
    endif(HAVE_POSIX_FALLOCATE)
  endif(HAVE_FALLOCATE)

  target_link_libraries(isisImageFormat_tar_proxy isis_core ${ISIS_LIB_DEPENDS} ${Boost_IOSTREAMS_LIBRARY} ${LIB_Z} ${LIB_BZ2})
  set(TARGETS ${TARGETS} isisImageFormat_tar_proxy)
endif(${CMAKE_PROJECT_NAME}_IOPLUGIN_TAR)

############################################################
# Dicom
############################################################
if(${CMAKE_PROJECT_NAME}_IOPLUGIN_DICOM)
  find_package(DCMTK)
  include_directories(${DCMTK_INCLUDE_DIRS})

  find_library(LIB_Z z)
  find_library(LIB_TIFF tiff)
  find_library(LIB_PNG png12)

  add_library(isisImageFormat_Dicom SHARED imageFormat_Dicom.cpp imageFormat_DicomParser.cpp)
  target_link_libraries(isisImageFormat_Dicom 
    ${DCMTK_dcmdata_LIBRARY} ${DCMTK_dcmimgle_LIBRARY} ${DCMTK_ofstd_LIBRARY} ${DCMTK_dcmimage_LIBRARY} 
    ${LIB_Z} ${LIB_TIFF} ${LIB_PNG}
    isis_core 
    ${ISIS_LIB_DEPENDS}
  )
# 
  set(TARGETS ${TARGETS} isisImageFormat_Dicom)
endif(${CMAKE_PROJECT_NAME}_IOPLUGIN_DICOM)

############################################################
# RAW plugin
############################################################
if(${CMAKE_PROJECT_NAME}_IOPLUGIN_RAW)
  add_library(isisImageFormat_raw SHARED imageFormat_raw.cpp)
  target_link_libraries(isisImageFormat_raw isis_core ${ISIS_LIB_DEPENDS})
  set(TARGETS ${TARGETS} isisImageFormat_raw)
endif(${CMAKE_PROJECT_NAME}_IOPLUGIN_RAW)

###########################################################################
# prepare all plugins for installation
###########################################################################
message(STATUS "Installing following plugins to ${CMAKE_INSTALL_PREFIX}/${ISIS_PLUGIN_INFIX}")

foreach(PLUGIN ${TARGETS})
  message(STATUS " == ${PLUGIN}")
  set_target_properties(${PLUGIN} PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR} INSTALL_RPATH_USE_LINK_PATH TRUE)
endforeach(PLUGIN)

###########################################################
# actual install
###########################################################
install(TARGETS ${TARGETS} DESTINATION ${ISIS_PLUGIN_INFIX} COMPONENT "IO plugins" )

# # uninstall target
# configure_file(
# 	"${CMAKE_CURRENT_SOURCE_DIR}/cmake/cmake_uninstall.cmake.in"
# 	"${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
# 	IMMEDIATE @ONLY
# )

