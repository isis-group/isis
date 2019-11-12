############################################################
# build the core library
############################################################
find_library(LIB_JSONCPP jsoncpp)
find_path(INC_JSONCPP "json/config.h" PATH_SUFFIXES "jsoncpp" DOC "path where json/config.h can be found")
include_directories(${INC_JSONCPP})

############################################################
# check for ncurses
############################################################
find_package(Curses QUIET)
if(CURSES_LIBRARIES)
	include_directories(${CURSES_CURSES_H_PATH})
	add_definitions(-DHAVE_CURSES)
	message(STATUS "curses form libraries ${CURSES_LIBRARIES} found. Enable colored logging.")
else(CURSES_LIBRARIES)
	message(STATUS "curses form library not found. Logging won't be colored.")
	set(CURSES_LIBRARIES "")
endif(CURSES_LIBRARIES)
 
# add source files to this parameter list
file(GLOB CORE_SRC_FILES "*.cpp")
file(GLOB CORE_HDR_FILES "*.hpp" "*.h")

############################################################
# check for liborc
############################################################
set(ISIS_USE_LIBORC OFF CACHE BOOL "use the \"Oil Runtime Compiler\"" )
if(ISIS_USE_LIBORC)
	find_file(LIBORC_H "orc/orc.h" DOC "Where to find the orc include files (looks for orc/orc.h)" )
	find_library(LIBORC_LIB "orc-0.4")
	
	find_program(LIBORC_COMPILER "orcc")

	ADD_CUSTOM_COMMAND(
		COMMAND ${LIBORC_COMPILER} ARGS --implementation ${CMAKE_CURRENT_SOURCE_DIR}/type_conversions.orc -o type_conversions.c
		COMMAND ${LIBORC_COMPILER} ARGS --header ${CMAKE_CURRENT_SOURCE_DIR}/type_conversions.orc -o type_conversions.h
		DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/type_conversions.orc
		WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
		OUTPUT ${CMAKE_BINARY_DIR}/type_conversions.c
	)
	
	include_directories(${LIBORC_H})
	include_directories(${CMAKE_BINARY_DIR})#so we can #include the generated orc-code
	list(APPEND CORE_SRC_FILES ${CMAKE_BINARY_DIR}/type_conversions.c)
else(ISIS_USE_LIBORC)
	add_definitions(-DDISABLE_ORC)
	set(LIBORC_LIB "")
endif(ISIS_USE_LIBORC)

#add the core library
add_lib(isis_core
	"${CORE_SRC_FILES}"
	"${Boost_LIBRARIES};${CURSES_LIBRARIES}" #deps
	"${LIBORC_LIB};${CMAKE_DL_LIBS};${LIB_JSONCPP};Threads::Threads" #private deps
	${ISIS_CORE_VERSION_SO} ${ISIS_CORE_VERSION_API}
)

############################################################
# set the plugin directory and the system dependend shared
# library naming conventions for data/io_factory.cpp
############################################################
if(WIN32)
set_source_files_properties( "io_factory.cpp" PROPERTIES COMPILE_FLAGS
	"-DDL_SUFFIX=\\\"${CMAKE_SHARED_LIBRARY_SUFFIX}\\\" -DDL_PREFIX=\\\"${CMAKE_SHARED_LIBRARY_PREFIX}\\\""
)
else(WIN32)
set_source_files_properties( "io_factory.cpp" PROPERTIES COMPILE_FLAGS
	"-DPLUGIN_PATH=\\\"${CMAKE_INSTALL_PREFIX}/${ISIS_PLUGIN_INFIX}\\\" -DDL_SUFFIX=\\\"${CMAKE_SHARED_LIBRARY_SUFFIX}\\\" -DDL_PREFIX=\\\"${CMAKE_SHARED_LIBRARY_PREFIX}\\\""
)
endif(WIN32)

############################################################
# Installation
############################################################

# install header files
install(FILES ${CORE_HDR_FILES} DESTINATION include/isis/core COMPONENT Development)


