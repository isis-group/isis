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
# set the plugin directory and the system dependend shared
# library naming conventions
############################################################
add_definitions(-DPLUGIN_PATH="${CMAKE_INSTALL_PREFIX}/${ISIS_PLUGIN_INFIX}")
add_definitions(-DDL_SUFFIX="${CMAKE_SHARED_LIBRARY_SUFFIX}")
add_definitions(-DDL_PREFIX="${CMAKE_SHARED_LIBRARY_PREFIX}")

############################################################
# export library dependencies of isis_core
############################################################
set(ISIS_LIB_DEPENDS  ${CMAKE_DL_LIBS} ${Boost_LIBRARIES} ${LibOil_LIBRARY}
  CACHE INTERNAL "Addition libraries ISIS depends on")

############################################################
# Build config
############################################################
set(ISIS_BUILD_PROPERTIES LINK_FLAGS "--no-undefined" CLEAN_DIRECT_OUTPUT 1)
set(ISIS_BUILD_STATIC ON CACHE BOOL "enable build of the static library" )
set(ISIS_BUILD_DYNAMIC ON CACHE BOOL "enable build of the dynamic library" )

############################################################
# build core
############################################################

# add source files to this parameter list
file(GLOB CORE_SRC_FILES "DataStorage/*.cpp" "CoreUtils/*.cpp")
file(GLOB COREUTILS_HDR_FILES "CoreUtils/*.hpp" "CoreUtils/*.h")
file(GLOB DATASTORAGE_HDR_FILES "DataStorage/*.hpp" "DataStorage/*.h")

# the core library shared and static build
if(ISIS_BUILD_DYNAMIC)
	set(Boost_USE_STATIC_LIBS OFF)
	set(LIBS isis_core ${LIBS})
	add_library( isis_core SHARED ${CORE_SRC_FILES} )
	target_link_libraries( isis_core ${CMAKE_DL_LIBS} ${Boost_LIBRARIES} ${LibOil_LIBRARY})
	set_target_properties( isis_core PROPERTIES	${ISIS_BUILD_PROPERTIES} VERSION ${${CMAKE_PROJECT_NAME}_VERSION} INSTALL_NAME_DIR "${CMAKE_INSTALL_PREFIX}/lib")
endif(ISIS_BUILD_DYNAMIC)

if(ISIS_BUILD_STATIC)
	set(LIBS isis_static ${LIBS})
	set(Boost_USE_STATIC_LIBS ON)
	add_library( isis_static STATIC ${CORE_SRC_FILES} )
	set_target_properties( isis_static PROPERTIES ${ISIS_BUILD_PROPERTIES} OUTPUT_NAME "isis_core" )
endif(ISIS_BUILD_STATIC)

############################################################
# Installation
############################################################

# install libraries
install (TARGETS ${LIBS}
  LIBRARY DESTINATION lib COMPONENT RuntimeLibraries
  ARCHIVE DESTINATION lib COMPONENT Development)

# install header files
install(FILES ${COREUTILS_HDR_FILES} DESTINATION include/isis/CoreUtils COMPONENT Development)
install(FILES ${DATASTORAGE_HDR_FILES} DESTINATION include/isis/DataStorage COMPONENT Development)

# write configuration file
configure_file(cmake/isis_corecfg.cmake.in ${CMAKE_BINARY_DIR}/ISISConfig.cmake @ONLY)

# install cmake configuration files
install(FILES ${CMAKE_BINARY_DIR}/ISISConfig.cmake DESTINATION share/isis/cmake COMPONENT Development)

# uninstall target
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/cmake//cmake_uninstall.cmake.in" "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake" IMMEDIATE @ONLY)

add_custom_target(uninstall COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake)
