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
# optional components
############################################################
option(ISIS_QT5 "Enable adapter to the Qt5 library" OFF)

if(ISIS_QT5)
	find_package(Qt5Widgets)

	set(CMAKE_AUTOMOC ON)
#	set(CMAKE_AUTOUIC ON) #doesn't work on older cmake

	include_directories(${Qt5Widgets_INCLUDE_DIRS})
	include_directories(${CMAKE_CURRENT_BINARY_DIR})

	add_definitions(${Qt5Widgets_DEFINITIONS})

	file(GLOB ISIS_QT5_LIBRARY_FILES "qt5/*.cpp")
	file(GLOB ISIS_QT5_HEADER_FILES "qt5/*.hpp")
	
	add_lib(isis_adapter_qt5
		"${ISIS_QT5_LIBRARY_FILES};${UI_FILE}"
		"" "${isis_core_lib};Qt5::Widgets"
		${ISIS_QT_VERSION_SO} ${ISIS_QT_VERSION_API}
	)

	# export header files
	install(FILES ${ISIS_QT5_HEADER_FILES} DESTINATION ${CMAKE_INSTALL_PREFIX}/include/isis/adapter/qt5 COMPONENT Development)
endif(ISIS_QT5)

option(ISIS_ITK "Enable adapter to the ITK library" OFF)

if(ISIS_ITK)
	find_package(ITK REQUIRED ITKCommon ITKTransform)
	include(${ITK_USE_FILE})
	
	file(GLOB ISIS_ITK_LIBRARY_FILES "itk/*.cpp")
	file(GLOB ISIS_ITK_HEADER_FILES "itk/*.hpp")

	add_lib(isis_adapter_itk
		"${ISIS_ITK_LIBRARY_FILES}"
		"" "${isis_core_lib};${isis_math_lib};${ITK_LIBRARIES}"
		"0.0.0" "0.0.0"
	)

	# export header files
	install(FILES ${ISIS_ITK_HEADER_FILES} DESTINATION ${CMAKE_INSTALL_PREFIX}/include/isis/adapter/qt5 COMPONENT Development)
endif(ISIS_ITK)

############################################################
# Install header and helper files
############################################################

# write configuration file
configure_file(isis_adaptercfg.cmake.in ${CMAKE_BINARY_DIR}/ISISAdapterConfig.cmake @ONLY)
install(FILES ${CMAKE_BINARY_DIR}/ISISAdapterConfig.cmake DESTINATION share/isis/cmake COMPONENT Development)
