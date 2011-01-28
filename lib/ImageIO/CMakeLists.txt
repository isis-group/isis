# the ISIS ImageIO plugins
CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

PROJECT(ISIS_IMAGE_IO)

###############################################################################
# add global options
###############################################################################

############################################################
# set isis version number
############################################################
SET(${CMAKE_PROJECT_NAME}_VERSION_MAJOR "0")
SET(${CMAKE_PROJECT_NAME}_VERSION_MINOR "1")
SET(${CMAKE_PROJECT_NAME}_VERSION_PATCH "0")

SET(${CMAKE_PROJECT_NAME}_VERSION
  ${${CMAKE_PROJECT_NAME}_VERSION_MAJOR}.${${CMAKE_PROJECT_NAME}_VERSION_MINOR}.${${CMAKE_PROJECT_NAME}_VERSION_PATCH})

###########################################################
# find and configure the isis core library
###########################################################
SET(ISIS_DIR /usr/share/isis/cmake CACHE PATH
    "The directory where the isis cmake configuration skript script is located.")
FIND_PACKAGE(ISIS REQUIRED)
INCLUDE_DIRECTORIES(${ISIS_INCLUDE_DIRS})

string(COMPARE EQUAL ${CMAKE_INSTALL_PREFIX} ${ISIS_INSTALL_PREFIX} PREFIX_IS_SAME)
if(NOT ${PREFIX_IS_SAME})
	message(SEND_ERROR
		"Seems like your install prefix (${CMAKE_INSTALL_PREFIX}) is not the same as the prefix, which was used when the core library was built (${ISIS_INSTALL_PREFIX})."
		"This will prevent this core library from finding the plugins you are building right now. Please use CMAKE_INSTALL_PREFIX=${ISIS_INSTALL_PREFIX}"
	)
endif(NOT ${PREFIX_IS_SAME})

###############################################################################
# add global options
###############################################################################

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


OPTION(${CMAKE_PROJECT_NAME}_IOPLUGIN_NIFTI "Enable Nifti-IO plugin" OFF)
OPTION(${CMAKE_PROJECT_NAME}_IOPLUGIN_NULL "Enable Null-IO plugin" ON)
OPTION(${CMAKE_PROJECT_NAME}_IOPLUGIN_PNG "Enable Null-IO plugin" OFF)
OPTION(${CMAKE_PROJECT_NAME}_IOPLUGIN_DICOM "Enable Dicom-IO plugin" OFF)
OPTION(${CMAKE_PROJECT_NAME}_IOPLUGIN_VISTA "Enable Vista-IO plugin" OFF)
OPTION(${CMAKE_PROJECT_NAME}_IOPLUGIN_GZ "Enable proxy plugin for compressed files" ON)
OPTION(${CMAKE_PROJECT_NAME}_IOPLUGIN_TAR "Enable proxy plugin for tar datasets" ON)
OPTION(${CMAKE_PROJECT_NAME}_IOPLUGIN_RAW "Enable plugin for raw data output" ON)
OPTION(${CMAKE_PROJECT_NAME}_IOPLUGIN_SIEMENSTCPIP "Enable plugin for Siemens data coming on TCP Port" OFF)


#OPTION(${CMAKE_PROJECT_NAME}_PACKAGE_RPM "Enable support for RPM packaging" OFF)

############################################################
# PACKAGING
# @TODO: Check if CPACK is a better solution for building 
# a rpm package
############################################################
#IF(${CMAKE_PROJECT_NAME}_PACKAGE_RPM)
#
#    MESSAGE(STATUS "Configuring RPM build")
#
#    # write spec file for rpm package
#    CONFIGURE_FILE(cmake/template.spec.in ${CMAKE_BINARY_DIR}/isisio0.spec
#      @ONLY)
#    CONFIGURE_FILE(cmake/bootstrap_rpm.sh.in ${CMAKE_BINARY_DIR}/bootstrap_rpm.sh
#      @ONLY)
#
#    # add a custom make target
#    ADD_CUSTOM_TARGET(rpm   
#        "${CMAKE_BINARY_DIR}/bootstrap_rpm.sh" WORKING_DIRECTORY ${CMAKE_BINARY_DIR})
#
#ENDIF()

#############################################################################################
# the plugins ...
#############################################################################################

# Nifti
if(${CMAKE_PROJECT_NAME}_IOPLUGIN_NIFTI)
	find_library(LIB_NIFTIIO niftiio)
	find_library(LIB_ZNZ znz)
	find_library(LIB_Z z)

	find_path(INCPATH_NIFTI "nifti1_io.h" PATH_SUFFIXES "nifti")
	find_path(INCPATH_ZNZ "znzlib.h" PATH_SUFFIXES "nifti")

	include_directories(${INCPATH_NIFTI} ${INCPATH_ZNZ})
	add_library(isisImageFormat_Nifti SHARED imageFormat_Nifti.cpp)
	target_link_libraries(isisImageFormat_Nifti ${LIB_NIFTIIO} ${LIB_ZNZ} ${LIB_Z} ${ISIS_LIB} ${ISIS_LIB_DEPENDS})

	SET(TARGETS ${TARGETS} isisImageFormat_Nifti)
endif(${CMAKE_PROJECT_NAME}_IOPLUGIN_NIFTI)

# vista
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
    IF(VIA_ROOT_INCLUDE_DIR)
      INCLUDE_DIRECTORIES(${VIA_ROOT_INCLUDE_DIR})
    ENDIF(VIA_ROOT_INCLUDE_DIR)

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
        TARGET_LINK_LIBRARIES(isisImageFormat_Vista ${VIAIO_LIBRARY}  ${ISIS_LIB} ${ISIS_LIB_DEPENDS})
        SET(TARGETS ${TARGETS} isisImageFormat_Vista)
    else(VIA_INCLUDE_DIR AND VIAIO_INCLUDE_DIR
        AND VIA_LIBRARY AND VIAIO_LIBRARY)

        MESSAGE(FATAL_ERROR, "Cannot find viaiio libraries. Please install the package libvia-dev")

    endif(VIA_INCLUDE_DIR AND VIAIO_INCLUDE_DIR
        AND VIA_LIBRARY AND VIAIO_LIBRARY)
endif(${CMAKE_PROJECT_NAME}_IOPLUGIN_VISTA)

# null io-plugin
if(${CMAKE_PROJECT_NAME}_IOPLUGIN_NULL)
	add_library(isisImageFormat_Null SHARED imageFormat_Null.cpp)
	target_link_libraries(isisImageFormat_Null ${ISIS_LIB} ${ISIS_LIB_DEPENDS})

	SET(TARGETS ${TARGETS} isisImageFormat_Null)
endif(${CMAKE_PROJECT_NAME}_IOPLUGIN_NULL)

# png
if(${CMAKE_PROJECT_NAME}_IOPLUGIN_PNG)
	find_library(LIB_Z z)
	find_library(LIB_PNG png12)

	find_path(INCPATH_PNG "png.h")
	include_directories(${INCPATH_PNG})

	add_library(isisImageFormat_png SHARED imageFormat_png.cpp)
	target_link_libraries(isisImageFormat_png ${LIB_Z} ${ISIS_LIB} ${ISIS_LIB_DEPENDS} ${LIB_PNG})

	SET(TARGETS ${TARGETS} isisImageFormat_png)
endif(${CMAKE_PROJECT_NAME}_IOPLUGIN_PNG)


# gz proxy
if(${CMAKE_PROJECT_NAME}_IOPLUGIN_GZ)
	FIND_LIBRARY(LIB_Z "z")
	FIND_PATH(INCPATH_GZIP "zlib.h")
	INCLUDE_DIRECTORIES(${INCPATH_GZIP})

	add_library(isisImageFormat_gz_proxy SHARED imageFormat_gz_proxy.cpp)
	target_link_libraries(isisImageFormat_gz_proxy ${LIB_Z} ${ISIS_LIB} ${ISIS_LIB_DEPENDS})

	SET(TARGETS ${TARGETS} isisImageFormat_gz_proxy)
endif(${CMAKE_PROJECT_NAME}_IOPLUGIN_GZ)

# tar proxy
if(${CMAKE_PROJECT_NAME}_IOPLUGIN_TAR)
	include(CheckFunctionExists)
	FIND_PACKAGE(Boost REQUIRED COMPONENTS iostreams)

	add_library(isisImageFormat_tar_proxy SHARED imageFormat_tar_proxy.cpp)

	check_function_exists(fallocate HAVE_FALLOCATE)
	if(HAVE_FALLOCATE)
		set_target_properties(isisImageFormat_tar_proxy PROPERTIES COMPILE_DEFINITIONS HAVE_FALLOCATE)
	else(HAVE_FALLOCATE)
        	check_function_exists(posix_fallocate HAVE_POSIX_FALLOCATE)
		if(HAVE_POSIX_FALLOCATE)
			set_target_properties(isisImageFormat_tar_proxy PROPERTIES COMPILE_DEFINITIONS HAVE_POSIX_FALLOCATE)
		endif(HAVE_POSIX_FALLOCATE)
	endif(HAVE_FALLOCATE)

	target_link_libraries(isisImageFormat_tar_proxy ${ISIS_LIB} ${ISIS_LIB_DEPENDS} ${Boost_IOSTREAMS_LIBRARY})
	SET(TARGETS ${TARGETS} isisImageFormat_tar_proxy)
endif(${CMAKE_PROJECT_NAME}_IOPLUGIN_TAR)

# Dicom
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
	target_link_libraries(isisImageFormat_Dicom ${LIB_DCMDATA} ${LIB_DCMIMGLE} ${LIB_OFSTD} ${LIB_Z} ${ISIS_LIB} ${ISIS_LIB_DEPENDS} ${LIB_DCMIMAGE} ${LIB_TIFF} ${LIB_PNG})

	SET(TARGETS ${TARGETS} isisImageFormat_Dicom)
endif(${CMAKE_PROJECT_NAME}_IOPLUGIN_DICOM)


#################################################################################################
# RawP
#################################################################################################

if(${CMAKE_PROJECT_NAME}_IOPLUGIN_RAW)
	add_library(isisImageFormat_raw SHARED imageFormat_raw.cpp)
	target_link_libraries(isisImageFormat_raw ${ISIS_LIB} ${ISIS_LIB_DEPENDS})
	SET(TARGETS ${TARGETS} isisImageFormat_raw)
endif(${CMAKE_PROJECT_NAME}_IOPLUGIN_RAW)


#################################################################################################
# Siemens TCPIP
#################################################################################################

if (${CMAKE_PROJECT_NAME}_IOPLUGIN_SIEMENSTCPIP)
	add_library(isisImageFormat_SiemensTcpIp SHARED imageFormat_SiemensTcpIp.cpp)
	target_link_libraries(isisImageFormat_SiemensTcpIp ${ISIS_LIB} ${ISIS_LIB_DEPENDS})
	SET(TARGETS ${TARGETS} isisImageFormat_SiemensTcpIp)
endif(${CMAKE_PROJECT_NAME}_IOPLUGIN_SIEMENSTCPIP)


###########################################################################
# prepare all plugins for installation
###########################################################################
MESSAGE(STATUS "Installing following plugins to ${CMAKE_INSTALL_PREFIX}/${ISIS_PLUGIN_INFIX}")

foreach(PLUGIN ${TARGETS})
	MESSAGE(STATUS " == ${PLUGIN}")
	set_target_properties(${PLUGIN} PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR} INSTALL_RPATH_USE_LINK_PATH TRUE)
endforeach(PLUGIN)

###########################################################
# CPack config
###########################################################
SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY "isis default io plugins")
SET(CPACK_PACKAGE_VENDOR "the friendly isis team")
SET(CPACK_PACKAGE_CONTACT "reimer@cbs.mpg.de")
SET(CPACK_PACKAGE_NAME "isis-io")
SET(CPACK_BUNDLE_NAME ${CPACK_PACKAGE_NAME})
#SET(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_CURRENT_SOURCE_DIR}/ReadMe.txt")
SET(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/../../COPYING.txt")
SET(CPACK_PACKAGE_VERSION_MAJOR ${${CMAKE_PROJECT_NAME}_VERSION_MAJOR})
SET(CPACK_PACKAGE_VERSION_MINOR ${${CMAKE_PROJECT_NAME}_VERSION_MINOR})
SET(CPACK_PACKAGE_VERSION_PATCH ${${CMAKE_PROJECT_NAME}_VERSION_PATCH})
SET(CPACK_PACKAGE_FILE_NAME ${CPACK_PACKAGE_NAME}-${ISIS_CORE_REV_STRING}-${CMAKE_SYSTEM_NAME}-${CMAKE_SYSTEM_PROCESSOR}-${CMAKE_BUILD_TYPE})
#SET(CPACK_PACKAGE_INSTALL_DIRECTORY "CMake ${${CMAKE_PROJECT_NAME}_VERSION_MAJOR}.${${CMAKE_PROJECT_NAME}_VERSION_MINOR}")
INCLUDE(CPack)


###########################################################
# actual install
###########################################################
INSTALL(TARGETS ${TARGETS}
	LIBRARY DESTINATION ${ISIS_PLUGIN_INFIX} 
	RUNTIME DESTINATION ${ISIS_PLUGIN_INFIX} 
	ARCHIVE DESTINATION ${ISIS_PLUGIN_INFIX}
	COMPONENT RuntimeLibraries
)

# uninstall target
configure_file(
	"${CMAKE_CURRENT_SOURCE_DIR}/cmake/cmake_uninstall.cmake.in"
	"${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
	IMMEDIATE @ONLY
)

add_custom_target(uninstall COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake)
