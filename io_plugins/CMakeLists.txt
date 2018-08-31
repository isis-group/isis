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
set_directory_properties(PROPERTIES INCLUDE_DIRECTORIES ${CMAKE_SOURCE_DIR})

############################################################
# add global options
############################################################
option(ISIS_IOPLUGIN_NIFTI_SA "Enable standalone Nifti-IO plugin" ON)
option(ISIS_IOPLUGIN_TIFF_SA "Enable standalone TIFF-IO plugin" ON)
option(ISIS_IOPLUGIN_ZISRAW "Enable standalone Zeiss Integrated Software RAW plugin" ON)
option(ISIS_IOPLUGIN_MEAS "Enable standalone plugin for reading siemens RAW data files" OFF)
option(ISIS_IOPLUGIN_ORTHANC "Enable standalone plugin for reading siemens RAW data files" OFF)
option(ISIS_IOPLUGIN_VISTA_SA "Enable standalone Vista-IO plugin" OFF)
option(ISIS_IOPLUGIN_NULL "Enable Null-IO plugin" OFF)
option(ISIS_IOPLUGIN_PNG "Enable PNG-IO plugin" OFF)
option(ISIS_IOPLUGIN_IMAGEMAGIC "Enable plugin for ImageMagic based IO" OFF)
option(ISIS_IOPLUGIN_DICOM "Enable Dicom-IO plugin" OFF)
option(ISIS_IOPLUGIN_COMP "Enable proxy plugin for compressed data (gz, bzip2 and Z)" ON)
option(ISIS_IOPLUGIN_TAR "Enable proxy plugin for reading tar archives" ON)
option(ISIS_IOPLUGIN_FDF "Enable plugin for reading fdf files" ON)
option(ISIS_IOPLUGIN_FLIST "Enable proxy plugin which gets filenames from a file or stdin" ON)
option(ISIS_IOPLUGIN_SIEMENSTCPIP "Enable plugin for Siemens data coming on TCP Port" OFF)

############################################################
# the plugins ...
############################################################

############################################################
# fdf plugin
############################################################
if(ISIS_IOPLUGIN_FDF)
	add_library(isisImageFormat_fdf SHARED imageFormat_fdf.cpp )
	target_link_libraries(isisImageFormat_fdf ${isis_core_lib})
	set(TARGETS ${TARGETS} isisImageFormat_fdf)
endif(ISIS_IOPLUGIN_FDF)

############################################################
# NIFTI plugin
############################################################
if(ISIS_IOPLUGIN_NIFTI_SA)
	add_library(isisImageFormat_Nifti_sa SHARED imageFormat_nifti_sa.cpp imageFormat_nifti_dcmstack.cpp )
	target_link_libraries(isisImageFormat_Nifti_sa ${isis_core_lib} ${isis_math_lib})
	set(TARGETS ${TARGETS} isisImageFormat_Nifti_sa)
endif(ISIS_IOPLUGIN_NIFTI_SA)

############################################################
# meas plugin
############################################################
if(ISIS_IOPLUGIN_MEAS)
	add_library(isisImageFormat_meas SHARED imageFormat_meas.cpp )
	target_link_libraries(isisImageFormat_meas ${isis_core_lib})
	set(TARGETS ${TARGETS} isisImageFormat_meas)
endif(ISIS_IOPLUGIN_MEAS)

############################################################
# meas plugin
############################################################
if(ISIS_IOPLUGIN_ORTHANC)
	find_package(Threads)

	add_library(isisImageFormat_orthanc SHARED imageFormat_orthanc.cpp )
	set_target_properties(isisImageFormat_orthanc PROPERTIES INCLUDE_DIRECTORIES ${INC_JSONCPP})
	target_link_libraries(isisImageFormat_orthanc ${isis_core_lib} ACE ACE_INet jsoncpp Threads::Threads)
	set(TARGETS ${TARGETS} isisImageFormat_orthanc)
endif(ISIS_IOPLUGIN_ORTHANC)

############################################################
# TIFF plugin
############################################################
if(ISIS_IOPLUGIN_TIFF_SA)
	find_path(TJPEG_INCLUDE_DIR "turbojpeg.h")
	find_library(TJPEG_LIBRARY "turbojpeg")

	add_library(isisImageFormat_tiff_sa SHARED imageFormat_tiff_sa.cpp )
	set_target_properties(isisImageFormat_tiff_sa PROPERTIES INCLUDE_DIRECTORIES ${TJPEG_INCLUDE_DIR})
	target_link_libraries(isisImageFormat_tiff_sa ${TJPEG_LIBRARY} ${isis_core_lib})
	set(TARGETS ${TARGETS} isisImageFormat_tiff_sa)
endif(ISIS_IOPLUGIN_TIFF_SA)

if(ISIS_IOPLUGIN_ZISRAW)
	find_path(JXR_INCLUDE_DIR "JXRGlue.h" PATH_SUFFIXES "jxrlib" DOC "libjxr include path")

	find_path(JXR_WINDOWSMEDIAPHOTO_DIR "windowsmediaphoto.h" PATH_SUFFIXES "image/sys" HINTS ${JXR_INCLUDE_DIR} DOC "path where to find windowsmediaphoto.h")
	find_path(JXR_WMSPECSTRING_DIR "wmspecstring.h" PATH_SUFFIXES "common/include" HINTS ${JXR_INCLUDE_DIR} DOC "path where to find wmspecstring.h")

	find_library(JXR_GLUE_LIBRARY "jxrglue")
	find_library(JXR_LIBRARY "jpegxr")
	
	add_library(isisImageFormat_ZISRAW SHARED imageFormat_ZISRAW.cpp imageFormat_ZISRAW_jxr.c)
	set_target_properties(isisImageFormat_ZISRAW PROPERTIES INCLUDE_DIRECTORIES ${JXR_INCLUDE_DIR} ${JXR_WINDOWSMEDIAPHOTO_DIR} ${JXR_WMSPECSTRING_DIR})
	
	target_link_libraries(isisImageFormat_ZISRAW ${isis_core_lib} ${JXR_GLUE_LIBRARY} ${JXR_LIBRARY})
	set(TARGETS ${TARGETS} isisImageFormat_ZISRAW)
endif(ISIS_IOPLUGIN_ZISRAW)

if(ISIS_IOPLUGIN_VISTA_SA)
	option(REGEN_PARSER "Regenerate the parser files" OFF)
	if(REGEN_PARSER)
		find_program(FLEXc++_EXECUTABLE "flexc++")
		find_program(BISONc++_EXECUTABLE "bisonc++")

		ADD_CUSTOM_COMMAND(
			COMMAND ${FLEXc++_EXECUTABLE} 
			ARGS --target-directory=${CMAKE_CURRENT_SOURCE_DIR}/Vista_sa/ ${CMAKE_CURRENT_SOURCE_DIR}/Vista_sa/lexer.l
			DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/Vista_sa/lexer.l
			OUTPUT 
				${CMAKE_CURRENT_SOURCE_DIR}/Vista_sa/lex.cc 
				${CMAKE_CURRENT_SOURCE_DIR}/Vista_sa/VistaScanner.ih
				${CMAKE_CURRENT_SOURCE_DIR}/Vista_sa/VistaScannerbase.h
		)

		ADD_CUSTOM_COMMAND(
			COMMAND ${BISONc++_EXECUTABLE} 
			ARGS --target-directory=${CMAKE_CURRENT_SOURCE_DIR}/Vista_sa/ ${CMAKE_CURRENT_SOURCE_DIR}/Vista_sa/grammar.y
			DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/Vista_sa/lex.cc ${CMAKE_CURRENT_SOURCE_DIR}/Vista_sa/grammar.y
			OUTPUT 
				${CMAKE_CURRENT_SOURCE_DIR}/Vista_sa/parse.cc
				${CMAKE_CURRENT_SOURCE_DIR}/Vista_sa/VistaParser.ih
				${CMAKE_CURRENT_SOURCE_DIR}/Vista_sa/VistaParserbase.h
		)
	endif(REGEN_PARSER)

	add_library(isisImageFormat_Vista_sa SHARED 
		Vista_sa/imageFormat_Vista_sa.cpp Vista_sa/vistaprotoimage.cpp 
		Vista_sa/lex.cc Vista_sa/parse.cc
	)
	target_link_libraries(isisImageFormat_Vista_sa ${isis_core_lib})
	set(TARGETS ${TARGETS} isisImageFormat_Vista_sa)
endif(ISIS_IOPLUGIN_VISTA_SA)


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

	add_library(isisImageFormat_png SHARED imageFormat_png.cpp)
	target_link_libraries(isisImageFormat_png ${isis_core_lib} PNG::PNG)

	set(TARGETS ${TARGETS} isisImageFormat_png)
endif(ISIS_IOPLUGIN_PNG)

############################################################
# ImageMagic plugin
############################################################
if(ISIS_IOPLUGIN_IMAGEMAGIC)
	find_package(ImageMagick REQUIRED Magick++ MagickWand MagickCore)

	add_library(isisImageFormat_ImageMagick SHARED imageFormat_ImageMagick.cpp)
	set_target_properties(isisImageFormat_ImageMagick PROPERTIES INCLUDE_DIRECTORIES ${ImageMagick_INCLUDE_DIRS})
	target_link_libraries(isisImageFormat_ImageMagick ${isis_core_lib} ${ImageMagick_LIBRARIES})

	add_definitions("-DMAGICKCORE_HDRI_ENABLE=0" "-DMAGICKCORE_QUANTUM_DEPTH=16")

	set(TARGETS ${TARGETS} isisImageFormat_ImageMagick)
endif(ISIS_IOPLUGIN_IMAGEMAGIC)

############################################################
# COMP proxy plugin
############################################################
if(ISIS_IOPLUGIN_COMP)
	option(ISIS_IOPLUGIN_COMP_LZMA "Add support for lzma in the plugin for compressed data (xz)" OFF)

	find_library(LIB_BZ2 "bz2")
	find_package(ZLIB)

	list(APPEND COMP_LIBS ${ZLIB_LIBRARIES} ${LIB_BZ2})
	list(APPEND COMP_SRC imageFormat_compressed.cpp)

	add_library(isisImageFormat_comp_proxy SHARED ${COMP_SRC})

	if(ISIS_IOPLUGIN_COMP_LZMA)
		find_library(LIB_LZMA "lzma")
		find_path(INCPATH_LZMA "lzma.h")

		set_target_properties(isisImageFormat_comp_proxy PROPERTIES INCLUDE_DIRECTORIES ${INCPATH_LZMA} COMPILE_DEFINITIONS "HAVE_LZMA")

		list(APPEND COMP_LIBS ${LIB_LZMA})
		list(APPEND COMP_SRC imageFormat_compressed_lzma.cpp)
	endif(ISIS_IOPLUGIN_COMP_LZMA)

	target_link_libraries(isisImageFormat_comp_proxy ${isis_core_lib} ${COMP_LIBS})
	set(TARGETS ${TARGETS} isisImageFormat_comp_proxy)
endif(ISIS_IOPLUGIN_COMP)

############################################################
# COMP proxy plugin
############################################################
if(ISIS_IOPLUGIN_TAR)
  add_library(isisImageFormat_tar_proxy SHARED imageFormat_tar.cpp)
  target_link_libraries(isisImageFormat_tar_proxy ${isis_core_lib})
  set(TARGETS ${TARGETS} isisImageFormat_tar_proxy)
endif(ISIS_IOPLUGIN_TAR)

############################################################
# FLIST proxy plugin
############################################################
if(ISIS_IOPLUGIN_FLIST)
	add_library(isisImageFormat_flist SHARED imageFormat_flist.cpp)
	target_link_libraries(isisImageFormat_flist ${isis_core_lib})
	set(TARGETS ${TARGETS} isisImageFormat_flist)
endif(ISIS_IOPLUGIN_FLIST)

############################################################
# Dicom
############################################################
if(ISIS_IOPLUGIN_DICOM)
	find_package(DCMTK NO_MODULE)

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
