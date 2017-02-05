set(ISIS_BUILD_STATIC_LIBS OFF CACHE BOOL "build all libraries static. If OFF a dynamic build will be done." )


############################################################
# macros to generate libs
############################################################
macro( build_lib name type sources deps private_deps soversion version)
	message(STATUS "${name} will be built as ${type} and installed to ${CMAKE_INSTALL_PREFIX}/lib")

	#build the library as -shared or as -static
	add_library( ${name}-${type} ${type} ${sources} )
	target_link_libraries( ${name}-${type} PUBLIC ${deps} PRIVATE ${private_deps})
	set_target_properties( ${name}-${type} PROPERTIES SOVERSION ${soversion} VERSION ${version} OUTPUT_NAME ${name} INSTALL_NAME_DIR "${CMAKE_INSTALL_PREFIX}/lib")

	# install libraries
	install (TARGETS ${name}-${type}
		RUNTIME DESTINATION bin COMPONENT RuntimeLibraries
		LIBRARY DESTINATION lib COMPONENT RuntimeLibraries
		ARCHIVE DESTINATION lib COMPONENT Development
	)

	set(${name}_lib ${name}-${type} CACHE INTERNAL "internal name for ${name}" FORCE)
endmacro( build_lib )

macro( add_lib name sources deps private_deps soversion version)
	if(ISIS_BUILD_STATIC_AND_SHARED)
		build_lib(${name} STATIC "${sources}" "${deps}" "${private_deps}" ${soversion} ${version})
		build_lib(${name} SHARED "${sources}" "${deps}" "${private_deps}" ${soversion} ${version}) #this will cause the shared libs logic name  to be used in ${name}_lib
	else(ISIS_BUILD_STATIC_AND_SHARED)
		if(ISIS_BUILD_STATIC_LIBS)
			build_lib(${name} STATIC "${sources}" "${deps}" "${private_deps}" ${soversion} ${version})
		else(ISIS_BUILD_STATIC_LIBS)
			build_lib(${name} SHARED "${sources}" "${deps}" "${private_deps}" ${soversion} ${version})
		endif(ISIS_BUILD_STATIC_LIBS)
	endif(ISIS_BUILD_STATIC_AND_SHARED)
	
	foreach(_FILE ${deps})
		get_target_filename(${_FILE} ${name}-exported-deps)
	endforeach(_FILE ${deps})
	
	#add deps to exported-deps if they are dynamically linked
	foreach(_FILE ${${name}-exported-deps})
		string(REGEX MATCH "${CMAKE_STATIC_LIBRARY_SUFFIX}$" IS_STATIC ${_FILE})
		if(IS_STATIC)
#			message(STATUS "${_FILE} is a static dep of ${name}")
			list(REMOVE_ITEM ${name}-exported-deps ${_FILE})
		endif(IS_STATIC)
	endforeach(_FILE ${${name}-exported-deps})
	
	if(${name}-exported-deps)
		message(STATUS "${name} (${${name}_lib}) will have external dependencies (stored in \${${name}-exported-deps})")
		foreach(_FILE ${${name}-exported-deps})
			message(STATUS "== ${_FILE}")
		endforeach(_FILE ${${name}-exported-deps})
	endif(${name}-exported-deps)

endmacro( add_lib )

macro(get_target_filename _TARGET VAR)
	if(TARGET ${_TARGET})
#		message(STATUS "make target ${_TARGET} into files")
		get_target_property(_SO_LOC ${_TARGET} IMPORTED_LOCATION_RELEASE)
		if(_SO_LOC)
			list(APPEND ${VAR} "${_SO_LOC}")
		endif(_SO_LOC)
	
		get_target_property(_SODEPS ${_TARGET} INTERFACE_LINK_LIBRARIES)
		foreach(_DEP ${_SODEPS})
			get_target_filename(${_DEP} ${VAR})
		endforeach(_DEP ${_SODEPS})

		list(REMOVE_DUPLICATES ${VAR})
	else(TARGET ${_TARGET})
		list(APPEND ${VAR} "${_TARGET}")
	endif(TARGET ${_TARGET})
endmacro(get_target_filename)

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

############################################################
# build the core library
############################################################
find_package(Threads)

# add source files to this parameter list
file(GLOB CORE_SRC_FILES "data/*.cpp" "util/*.cpp")
file(GLOB COREUTILS_HDR_FILES "util/*.hpp" "util/*.h")
file(GLOB DATASTORAGE_HDR_FILES "data/*.hpp" "data/*.h")

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

add_subdirectory(adapter)
add_subdirectory(math)

# write configuration file
configure_file(isis_corecfg.cmake.in ${CMAKE_BINARY_DIR}/ISISConfig.cmake @ONLY)
install(FILES ${CMAKE_BINARY_DIR}/ISISConfig.cmake DESTINATION share/isis/cmake COMPONENT Development)


