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
option(ISIS_QT4 "Enable adapter to the Qt4 library" OFF)
option(ISIS_QT5 "Enable adapter to the Qt5 library" OFF)

if(ISIS_QT4 AND ISIS_QT5)
	message(FATAL_ERROR "Cannot build support for qt4 and qt5 at the same time")
endif(ISIS_QT4 AND ISIS_QT5)

############################################################
# use QT4 component
############################################################
if(ISIS_QT4)
	# find qt4
	find_package(Qt4 REQUIRED)
	include(${QT_USE_FILE})

	qt4_wrap_cpp(qdefaultmessageprint_moc qt4/qdefaultmessageprint.hpp)
	# configure target

	set(ISIS_QT4_LIBRARY_FILES ${qdefaultmessageprint_moc}
						qt4/qdefaultmessageprint.cpp
						qt4/qtapplication.cpp
						qt4/qmatrixconversion.cpp )

	set(ISIS_QT4_HEADER_FILES qt4/qtapplication.hpp
								qt4/qdefaultmessageprint.hpp
								qt4/qmatrixconversion.hpp )

	add_lib(isisAdapter_qt4
		"${ISIS_QT4_LIBRARY_FILES}"
		"${isis_core_lib};${QT_LIBRARIES};${CMAKE_C_STANDARD_LIBRARIES}"
		${ISIS_QT_VERSION_SO} ${ISIS_QT_VERSION_API}
	)

  # export header files
  list(APPEND ADAPTER_HDR_FILES ${ISIS_QT4_HEADER_FILES} )
endif(ISIS_QT4)

if(ISIS_QT5)
	find_package(Qt5Widgets)

	include_directories(${Qt5Widgets_INCLUDE_DIRS})

	add_definitions(${Qt5Widgets_DEFINITIONS})

	qt5_wrap_cpp(qdefaultmessageprint_moc qt5/qdefaultmessageprint.hpp)

	set(ISIS_QT5_LIBRARY_FILES ${qdefaultmessageprint_moc} qt5/qdefaultmessageprint.cpp qt5/qtapplication.cpp qt5/qmatrixconversion.cpp)

	set(ISIS_QT5_HEADER_FILES qt5/qtapplication.hpp qt5/qdefaultmessageprint.hpp qt5/qmatrixconversion.hpp)

	add_lib(isisAdapter_qt5
		"${ISIS_QT5_LIBRARY_FILES}"
		"${isis_core_lib};Qt5::Widgets;${CMAKE_C_STANDARD_LIBRARIES}"
		${ISIS_QT_VERSION_SO} ${ISIS_QT_VERSION_API}
	)

	# export header files
	list(APPEND ADAPTER_HDR_FILES ${ISIS_QT5_HEADER_FILES} )
endif(ISIS_QT5)

############################################################
# Install header and helper files
############################################################
install(FILES 
  ${ADAPTER_HDR_FILES} DESTINATION ${CMAKE_INSTALL_PREFIX}/include/isis/Adapter COMPONENT Development)
