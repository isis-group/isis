############################################################
# build the core library
############################################################
find_package(Threads)

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

# add source files to this parameter list
file(GLOB CORE_SRC_FILES "*.cpp")
file(GLOB CORE_HDR_FILES "*.hpp" "*.h")

#add the core library
add_lib(isis_core
	"${CORE_SRC_FILES}"
	"${Boost_LIBRARIES};${CURSES_LIBRARIES}" #deps
	"${LibOil_LIBRARY};${CMAKE_DL_LIBS};${LIB_JSONCPP};Threads::Threads" #private deps
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


