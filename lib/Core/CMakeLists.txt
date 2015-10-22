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
# check for ncurses
############################################################
find_package(Curses QUIET)
if(CURSES_FORM_LIBRARY)
	include_directories(${CURSES_CURSES_H_PATH})
	add_definitions(-DHAVE_CURSES)
	message(STATUS "curses form library ${CURSES_FORM_LIBRARY} found. Enable colored logging.")
else(CURSES_FORM_LIBRARY)
	message(STATUS "curses form library not found. Logging won't be colored.")
	set(CURSES_FORM_LIBRARY "")
endif(CURSES_FORM_LIBRARY)


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
foreach(FILE ${CMAKE_DL_LIBS} ${Boost_LIBRARIES} ${LibOil_LIBRARY} ${CURSES_FORM_LIBRARY})
		string(REGEX MATCH "${CMAKE_SHARED_LIBRARY_SUFFIX}$" IS_SHARED ${FILE})
		if(IS_SHARED)
				LIST(APPEND ISIS_LIB_DEPENDS ${FILE})
		endif(IS_SHARED)
endforeach(FILE)
if(ISIS_LIB_DEPENDS)
	message(STATUS "${CMAKE_SYSTEM_NAME} core dependencies are:  ${ISIS_LIB_DEPENDS}")
else(ISIS_LIB_DEPENDS)
	message(STATUS "no core dependencies detected")
endif(ISIS_LIB_DEPENDS)


############################################################
# build the core library
############################################################

# add source files to this parameter list
file(GLOB CORE_SRC_FILES "data/*.cpp" "util/*.cpp")
file(GLOB COREUTILS_HDR_FILES "util/*.hpp" "util/*.h")
file(GLOB DATASTORAGE_HDR_FILES "data/*.hpp" "data/*.h")

#add the core library
add_lib(isis_core
	"${CORE_SRC_FILES}"
	"${CMAKE_DL_LIBS};${LibOil_LIBRARY};${Boost_LIBRARIES};${CURSES_FORM_LIBRARY}"
	${ISIS_CORE_VERSION_SO} ${ISIS_CORE_VERSION_API}
)

############################################################
# set the plugin directory and the system dependend shared
# library naming conventions for data/io_factory.cpp
############################################################
if(WIN32)
set_source_files_properties( "data/io_factory.cpp" PROPERTIES COMPILE_FLAGS
	"-DDL_SUFFIX=\\\"${CMAKE_SHARED_LIBRARY_SUFFIX}\\\" -DDL_PREFIX=\\\"${CMAKE_SHARED_LIBRARY_PREFIX}\\\""
)
else(WIN32)
set_source_files_properties( "data/io_factory.cpp" PROPERTIES COMPILE_FLAGS
	"-DPLUGIN_PATH=\\\"${CMAKE_INSTALL_PREFIX}/${ISIS_PLUGIN_INFIX}\\\" -DDL_SUFFIX=\\\"${CMAKE_SHARED_LIBRARY_SUFFIX}\\\" -DDL_PREFIX=\\\"${CMAKE_SHARED_LIBRARY_PREFIX}\\\""
)
endif(WIN32)

############################################################
# Installation
############################################################

# install header files
install(FILES "config.hpp" DESTINATION include/isis COMPONENT Development)
install(FILES ${COREUTILS_HDR_FILES} DESTINATION include/isis/util COMPONENT Development)
install(FILES ${DATASTORAGE_HDR_FILES} DESTINATION include/isis/data COMPONENT Development)

# write configuration file
configure_file(cmake/isis_corecfg.cmake.in ${CMAKE_BINARY_DIR}/ISISConfig.cmake @ONLY)

# install cmake configuration files
install(FILES ${CMAKE_BINARY_DIR}/ISISConfig.cmake DESTINATION share/isis/cmake COMPONENT Development)

