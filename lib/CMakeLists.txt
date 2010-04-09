# the ISIS library module

SET(PLUGIN_PATH ${PROJECT_BINARY_DIR} CACHE PATH "Path where the plugins shall be installed")

add_definitions(-DPLUGIN_PATH="${PLUGIN_PATH}")
add_definitions(-DDL_SUFFIX="${CMAKE_SHARED_LIBRARY_SUFFIX}")
add_definitions(-DDL_PREFIX="${CMAKE_SHARED_LIBRARY_PREFIX}")

IF(BUILD_COREUTILS)
	add_subdirectory(CoreUtils)
	if (BUILD_SHARED_LIBS)
		SET(ISIS_UTIL_LIBS utils)  
	else(BUILD_SHARED_LIBS)
		 SET(ISIS_UTIL_LIBS  utils ${Boost_LIBRARIES})  
	 endif(BUILD_SHARED_LIBS)
	 message(STATUS "ISIS_UTIL_LIBS is ${ISIS_UTIL_LIBS}")
ENDIF(BUILD_COREUTILS)

IF(BUILD_DATA_IO)
	add_subdirectory(DataStorage)
	SET(ISIS_DATA_LIBS data ${ISIS_UTIL_LIBS})
	MESSAGE(STATUS "ISIS_DATA_LIBS is ${ISIS_DATA_LIBS}")
	add_subdirectory(ImageIO)
ENDIF(BUILD_DATA_IO)

IF(HAVE_ITK)
	 include_directories(${ITK_INCLUDE_DIRS} itk)
	 add_subdirectory(extITK)
ENDIF(HAVE_ITK)

add_subdirectory(ExternalLibraryAdapter)
