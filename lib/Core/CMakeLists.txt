############################################################
# The ISIS project
# 
# CMake configuration file of the ISIS core library
#
# Author: Thomas Proeger <thomasproeger@googlemail.com>
# Date: Wed, 06 Apr 2011 14:01:10 +0200
# 
############################################################

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
set(ISIS_LIB_DEPENDS ${CMAKE_DL_LIBS} ${Boost_LIBRARIES} ${LibOil_LIBRARY} INTERNAL "Additional libraries to link when linking ISIS core")

############################################################
# Build config
############################################################
set(ISIS_BUILD_PROPERTIES CLEAN_DIRECT_OUTPUT 1)
set(ISIS_BUILD_STATIC OFF CACHE BOOL "enable build of the static library. If OFF a dynamic library will be build and linked." )

############################################################
# build core
############################################################

# add source files to this parameter list
file(GLOB CORE_SRC_FILES "DataStorage/*.cpp" "CoreUtils/*.cpp")
file(GLOB COREUTILS_HDR_FILES "CoreUtils/*.hpp" "CoreUtils/*.h")
file(GLOB DATASTORAGE_HDR_FILES "DataStorage/*.hpp" "DataStorage/*.h")

############################################################
# the core library shared and static build
############################################################
macro( build_core name type)
    add_library( ${name} ${type} ${CORE_SRC_FILES} )
	target_link_libraries( ${name} LINK_PRIVATE ${CMAKE_DL_LIBS} ${LibOil_LIBRARY} LINK_PUBLIC ${Boost_LIBRARIES})
    set_target_properties( ${name} PROPERTIES
		${ISIS_BUILD_PROPERTIES}
		SOVERSION ${ISIS_CORE_VERSION_SO}
		VERSION ${ISIS_CORE_VERSION_API}
		INSTALL_NAME_DIR "${CMAKE_INSTALL_PREFIX}/lib"
		OUTPUT_NAME "isis_core")
    set(target_list ${target_list} ${name})
endmacro( build_core )

#for packaging purpose we want to build both, shared and static with one build_command
#however this capability is not provided by an option in the cmake menu
if(ISIS_BUILD_STATIC_AND_SHARED)
    build_core( isis_core_static STATIC)
    build_core( isis_core SHARED)
else(ISIS_BUILD_STATIC_AND_SHARED)
    if(ISIS_BUILD_STATIC)
		build_core( isis_core STATIC)
    else(ISIS_BUILD_STATIC)
		build_core( isis_core SHARED)
    endif(ISIS_BUILD_STATIC)
endif(ISIS_BUILD_STATIC_AND_SHARED)


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

# install libraries
install (TARGETS ${target_list}
  RUNTIME DESTINATION bin COMPONENT RuntimeLibraries
  LIBRARY DESTINATION lib COMPONENT RuntimeLibraries
  ARCHIVE DESTINATION lib COMPONENT Development
)

# install header files
install(FILES "config.hpp" DESTINATION include/isis COMPONENT Development)
install(FILES ${COREUTILS_HDR_FILES} DESTINATION include/isis/CoreUtils COMPONENT Development)
install(FILES ${DATASTORAGE_HDR_FILES} DESTINATION include/isis/DataStorage COMPONENT Development)

# write configuration file
configure_file(cmake/isis_corecfg.cmake.in ${CMAKE_BINARY_DIR}/ISISConfig.cmake @ONLY)

# install cmake configuration files
install(FILES ${CMAKE_BINARY_DIR}/ISISConfig.cmake DESTINATION share/isis/cmake COMPONENT Development)

