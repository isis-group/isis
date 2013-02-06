############################################################
# The ISIS project
# 
# CMake configuration file of the ISIS core library
#
# Author: Thomas Proeger <thomasproeger@googlemail.com>
# Date: Wed, 06 Apr 2011 14:01:10 +0200
# 
############################################################
INCLUDE (CheckIncludeFiles)


############################################################
# check for byteswap intrinsics
############################################################
CHECK_INCLUDE_FILES(byteswap.h HAVE_BYTESWAP)

if(HAVE_BYTESWAP)
	add_definitions(-DHAVE_BYTESWAP)
endif(HAVE_BYTESWAP)

############################################################
# check for liboil
############################################################
set(ISIS_USE_LIBOIL OFF CACHE BOOL "use liboil" )
if(ISIS_USE_LIBOIL)
	find_path(LibOil_INCLUDE_DIR "liboil/liboil.h" PATH_SUFFIXES "liboil" "liboil-0.3")
	find_library(LibOil_LIBRARY NAMES "oil-0.3")
	include_directories(${LibOil_INCLUDE_DIR})
	add_definitions(-DISIS_USE_LIBOIL)
else(ISIS_USE_LIBOIL)
	set(LibOil_LIBRARY "")
endif(ISIS_USE_LIBOIL)

############################################################
# export library dependencies of isis_core
############################################################
set(ISIS_LIB_DEPENDS ${CMAKE_DL_LIBS} ${Boost_LIBRARIES} ${LibOil_LIBRARY})

############################################################
# build the core library
############################################################

# add source files to this parameter list
file(GLOB CORE_SRC_FILES "DataStorage/*.cpp" "CoreUtils/*.cpp")
file(GLOB COREUTILS_HDR_FILES "CoreUtils/*.hpp" "CoreUtils/*.h")
file(GLOB DATASTORAGE_HDR_FILES "DataStorage/*.hpp" "DataStorage/*.h")

#add the core library
add_lib(isis_core
	"${CORE_SRC_FILES}"
	"${CMAKE_DL_LIBS};${LibOil_LIBRARY};${Boost_LIBRARIES}"
	${ISIS_CORE_VERSION_SO} ${ISIS_CORE_VERSION_API}
)

############################################################
# set the plugin directory and the system dependend shared
# library naming conventions for DataStorage/io_factory.cpp
############################################################
if(WIN32)
set_source_files_properties( "DataStorage/io_factory.cpp" PROPERTIES COMPILE_FLAGS
	"-DDL_SUFFIX=\\\"${CMAKE_SHARED_LIBRARY_SUFFIX}\\\" -DDL_PREFIX=\\\"${CMAKE_SHARED_LIBRARY_PREFIX}\\\""
)
else(WIN32)
set_source_files_properties( "DataStorage/io_factory.cpp" PROPERTIES COMPILE_FLAGS
	"-DPLUGIN_PATH=\\\"${CMAKE_INSTALL_PREFIX}/${ISIS_PLUGIN_INFIX}\\\" -DDL_SUFFIX=\\\"${CMAKE_SHARED_LIBRARY_SUFFIX}\\\" -DDL_PREFIX=\\\"${CMAKE_SHARED_LIBRARY_PREFIX}\\\""
)
endif(WIN32)

############################################################
# Installation
############################################################

# install header files
install(FILES "config.hpp" DESTINATION include/isis COMPONENT Development)
install(FILES ${COREUTILS_HDR_FILES} DESTINATION include/isis/CoreUtils COMPONENT Development)
install(FILES ${DATASTORAGE_HDR_FILES} DESTINATION include/isis/DataStorage COMPONENT Development)

# write configuration file
configure_file(cmake/isis_corecfg.cmake.in ${CMAKE_BINARY_DIR}/ISISConfig.cmake @ONLY)

# install cmake configuration files
install(FILES ${CMAKE_BINARY_DIR}/ISISConfig.cmake DESTINATION share/isis/cmake COMPONENT Development)

