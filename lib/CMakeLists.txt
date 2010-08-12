# the ISIS library module

SET(ISIS_PLUGIN_PATH ${PROJECT_BINARY_DIR} CACHE PATH "Path where the plugins shall be installed")

IF(${CMAKE_PROJECT_NAME}_BUILD_COREUTILS)
	add_subdirectory(CoreUtils)
	if (BUILD_SHARED_LIBS)
		SET(ISIS_UTIL_LIBS utils)
	else(BUILD_SHARED_LIBS)
		 SET(ISIS_UTIL_LIBS  utils ${Boost_LIBRARIES})
	 endif(BUILD_SHARED_LIBS)
	 message(STATUS "ISIS_UTIL_LIBS is ${ISIS_UTIL_LIBS}")
ENDIF(${CMAKE_PROJECT_NAME}_BUILD_COREUTILS)

IF(${CMAKE_PROJECT_NAME}_BUILD_DATA_IO)
	add_definitions(-DPLUGIN_PATH="${ISIS_PLUGIN_PATH}")
	add_definitions(-DDL_SUFFIX="${CMAKE_SHARED_LIBRARY_SUFFIX}")
	add_definitions(-DDL_PREFIX="${CMAKE_SHARED_LIBRARY_PREFIX}")

	add_subdirectory(DataStorage)
	SET(ISIS_DATA_LIBS data ${ISIS_UTIL_LIBS})
	MESSAGE(STATUS "ISIS_DATA_LIBS is ${ISIS_DATA_LIBS}")
	add_subdirectory(ImageIO)
ENDIF(${CMAKE_PROJECT_NAME}_BUILD_DATA_IO)

configure_file("../cmake/isis_corecfg.cmake.in" "isis_corecfg.cmake")


IF(ITK_FOUND)
	 add_subdirectory(extITK)
ENDIF(ITK_FOUND)

add_subdirectory(Adapter)
