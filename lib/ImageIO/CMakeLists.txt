# the ISIS ImageIO plugins
CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

PROJECT(ISIS_IMAGE_IO)

###############################################################################
# add global options
###############################################################################

###########################################################
# find and configure the isis core library
###########################################################
SET(ISIS_DIR /usr/share/isis/cmake CACHE PATH 
    "The directory where the isis cmake configuration skript script is located.")
FIND_PACKAGE(ISIS REQUIRED)
INCLUDE_DIRECTORIES(${ISIS_INCLUDE_DIR})
LINK_DIRECTORIES(${ISIS_LIB_DIR})

# we use the log definitions of the core
# enable RUNTIME_LOG
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

OPTION(${CMAKE_PROJECT_NAME}_IOPLUGIN_NIFTI "Enable Nifti-IO plugin" OFF)
OPTION(${CMAKE_PROJECT_NAME}_IOPLUGIN_NULL "Enable Null-IO plugin" ON)
OPTION(${CMAKE_PROJECT_NAME}_IOPLUGIN_DICOM "Enable Dicom-IO plugin" OFF)
OPTION(${CMAKE_PROJECT_NAME}_IOPLUGIN_VISTA "Enable Vista-IO plugin" OFF)
OPTION(${CMAKE_PROJECT_NAME}_IOPLUGIN_GZ "Enable proxy plugin for compressed files" ON)

if(${CMAKE_PROJECT_NAME}_IOPLUGIN_NIFTI)
	find_library(LIB_NIFTIIO niftiio)
	find_library(LIB_ZNZ znz)
	find_path(INCPATH_NIFTI "nifti1_io.h" PATH_SUFFIXES "nifti")
	find_path(INCPATH_ZNZ "znzlib.h" PATH_SUFFIXES "nifti")

	include_directories(${INCPATH_NIFTI} ${INCPATH_ZNZ})
	add_library(isisImageFormat_Nifti SHARED imageFormat_Nifti.cpp)
	target_link_libraries(isisImageFormat_Nifti ${LIB_NIFTIIO} ${LIB_ZNZ} ${ISIS_LIB})

	set_target_properties(isisImageFormat_Nifti PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
	  INSTALL_RPATH_USE_LINK_PATH TRUE)
	SET(TARGETS ${TARGETS} isisImageFormat_Nifti)
endif(${CMAKE_PROJECT_NAME}_IOPLUGIN_NIFTI)

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

    # INCLUDE directories
    FIND_PATH(VIA_INCLUDE_DIR
        NAMES via.h
        PATH_SUFFIXES via
        DOC "The include directory containing via.h"
        )

    FIND_PATH(VIAIO_INCLUDE_DIR
        NAMES VImage.h
        PATH_SUFFIXES viaio
        DOC "The include directory containing VImage.h and option.h"
        )

    # LIBRARY files
    FIND_LIBRARY(VIA_LIBRARY
        NAMES via libvia
        DOC "The library file libvia.so"
    )

    FIND_LIBRARY(VIAIO_LIBRARY
        NAMES viaio libviaio
        DOC "The library file libviaio.so"
    )

    FIND_LIBRARY(VX_LIBRARY
        NAMES vx libvx
        DOC "The library file libvx.so"
    )

    # these variables are only visible in 'advanced mode'
    MARK_AS_ADVANCED(VIA_INCLUDE_DIR
        VIAIO_INCLUDE_DIR
        VIA_LIBRARY
        VIAIO_LIBRARY
        VX_LIBRARY
    )

    # if via components found configure build environment
    if(VIA_INCLUDE_DIR AND VIAIO_INCLUDE_DIR
        AND VIA_LIBRARY AND VIAIO_LIBRARY)

        INCLUDE_DIRECTORIES(${VIA_INCLUDE_DIR} ${VIAIO_INCLUDE_DIR})
        ADD_LIBRARY(isisImageFormat_Vista SHARED imageFormat_Vista.cpp)
        TARGET_LINK_LIBRARIES(isisImageFormat_Vista ${VIAIO_LIBRARY}  ${ISIS_LIB})
        SET_TARGET_PROPERTIES(isisImageFormat_Vista PROPERTIES
            LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
			INSTALL_RPATH_USE_LINK_PATH TRUE)

    else(VIA_INCLUDE_DIR AND VIAIO_INCLUDE_DIR
        AND VIA_LIBRARY AND VIAIO_LIBRARY)

        MESSAGE(FATAL_ERROR, "Cannot find viaiio libraries. Please install the package libvia-dev")

    endif(VIA_INCLUDE_DIR AND VIAIO_INCLUDE_DIR
        AND VIA_LIBRARY AND VIAIO_LIBRARY)

	  SET(TARGETS ${TARGETS} isisImageFormat_Vista)
endif(${CMAKE_PROJECT_NAME}_IOPLUGIN_VISTA)


if(${CMAKE_PROJECT_NAME}_IOPLUGIN_NULL)
	add_library(isisImageFormat_Null SHARED imageFormat_Null.cpp)
	target_link_libraries(isisImageFormat_Null ${ISIS_LIB})

	set_target_properties(isisImageFormat_Null PROPERTIES 
	  LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
	  INSTALL_RPATH_USE_LINK_PATH TRUE)
	SET(TARGETS ${TARGETS} isisImageFormat_Null)
endif(${CMAKE_PROJECT_NAME}_IOPLUGIN_NULL)

if(${CMAKE_PROJECT_NAME}_IOPLUGIN_GZ)
	FIND_LIBRARY(LIB_Z "z")
	FIND_PATH(INCPATH_GZIP "zlib.h")
	INCLUDE_DIRECTORIES(${GZIP_INC})

	add_library(isisImageFormat_gz_proxy SHARED isisImageFormat_gz_proxy.cpp)
	target_link_libraries(isisImageFormat_gz_proxy ${LIB_Z} ${ISIS_LIB})

	set_target_properties(isisImageFormat_gz_proxy PROPERTIES
	  LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
	  INSTALL_RPATH_USE_LINK_PATH TRUE)
	SET(TARGETS ${TARGETS} isisImageFormat_gz_proxy)
endif(${CMAKE_PROJECT_NAME}_IOPLUGIN_GZ)


if(${CMAKE_PROJECT_NAME}_IOPLUGIN_DICOM)
	find_library(LIB_DCMIMGLE dcmimgle)
	find_library(LIB_DCMIMAGE dcmimage)
	find_library(LIB_DCMDATA dcmdata)
	find_library(LIB_OFSTD ofstd)
	find_library(LIB_Z z)
	find_library(LIB_TIFF tiff)
	find_library(LIB_PNG png12)

	find_path(INCPATH_DCMTK "dcmtk/dcmdata/dcfilefo.h")
	include_directories(${INCPATH_DCMTK})

	add_library(isisImageFormat_Dicom SHARED imageFormat_Dicom.cpp imageFormat_DicomParser.cpp)
	target_link_libraries(isisImageFormat_Dicom ${LIB_DCMDATA} ${LIB_DCMIMGLE} ${LIB_OFSTD} ${LIB_Z} ${ISIS_LIB} ${LIB_DCMIMAGE} ${LIB_TIFF} ${LIB_PNG})

	set_target_properties(isisImageFormat_Dicom PROPERTIES
	  LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
	  INSTALL_RPATH_USE_LINK_PATH TRUE)
	SET(TARGETS ${TARGETS} isisImageFormat_Dicom)
endif(${CMAKE_PROJECT_NAME}_IOPLUGIN_DICOM)

MESSAGE(STATUS "Installing plugins to ${CMAKE_BINARY_DIR}")

INSTALL(TARGETS ${TARGETS}
  LIBRARY DESTINATION ${ISIS_PLUGIN_DIR} COMPONENT RuntimeLibraries)
