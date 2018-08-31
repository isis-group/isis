set(ISIS_BUILD_STATIC_LIBS OFF CACHE BOOL "build all libraries static. If OFF a dynamic build will be done." )

find_package(Threads)

############################################################
# macros to generate libs
############################################################
macro( build_lib name type sources deps private_deps soversion version)
	message(STATUS "${name} will be built as ${type} and installed to ${CMAKE_INSTALL_PREFIX}/lib")

	#build the library as -shared or as -static
	add_library( ${name}-${type} ${type} ${sources} )
	target_link_libraries( ${name}-${type} PUBLIC ${deps} PRIVATE ${private_deps})
	set_target_properties( ${name}-${type} PROPERTIES 
		SOVERSION ${soversion} VERSION ${version} 
		OUTPUT_NAME ${name} INSTALL_NAME_DIR "${CMAKE_INSTALL_PREFIX}/lib"
		INTERFACE_POSITION_INDEPENDENT_CODE "ON" #theoreticaly for isis_core and the plugins only (but as all others depend on isis_core it doesn't matter)
	)

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


############################################################
# Installation
############################################################

# install header files
install(FILES "config.hpp" DESTINATION include/isis COMPONENT Development)

add_subdirectory(core)
add_subdirectory(adapter)
add_subdirectory(math)

# write configuration file
configure_file(isis_corecfg.cmake.in ${CMAKE_BINARY_DIR}/ISISConfig.cmake @ONLY)
install(FILES ${CMAKE_BINARY_DIR}/ISISConfig.cmake DESTINATION share/isis/cmake COMPONENT Development)


