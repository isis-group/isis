############################################################
# The ISIS project
# 
# CMake configuration file for the lib subdir
#
# Author: Thomas Proeger <thomasproeger@googlemail.com>
# Date: Wed, 06 Apr 2011 12:51:43 +0200
# 
############################################################

set(ISIS_BUILD_STATIC_LIBS OFF CACHE BOOL "build all libraries static. If OFF a dynamic build will be done." )


############################################################
# macros to generate libs
############################################################
macro( build_lib name type sources deps soversion version)
	message(STATUS "${name} will be built as ${type} and installed to ${CMAKE_INSTALL_PREFIX}/lib")
	
	#build the library as -shared or as -static
	add_library( ${name}-${type} ${type} ${sources} )
	target_link_libraries( ${name}-${type} ${deps})
	set_target_properties( ${name}-${type} PROPERTIES SOVERSION ${soversion} VERSION ${version} OUTPUT_NAME ${name} INSTALL_NAME_DIR "${CMAKE_INSTALL_PREFIX}/lib")

	# install libraries
	install (TARGETS ${name}-${type}
		RUNTIME DESTINATION bin COMPONENT RuntimeLibraries
		LIBRARY DESTINATION lib${LIB_SUFFIX} COMPONENT RuntimeLibraries
		ARCHIVE DESTINATION lib${LIB_SUFFIX} COMPONENT Development
	)
	
	set(${name}_lib ${name}-${type} CACHE INTERNAL "internal name for ${name}" FORCE)
endmacro( build_lib )

macro( add_lib name sources deps soversion version)
	if(ISIS_BUILD_STATIC_AND_SHARED)
		build_lib(${name} STATIC "${sources}" "${deps}" ${soversion} ${version})
		build_lib(${name} SHARED "${sources}" "${deps}" ${soversion} ${version}) #this will cause the shared libs logic name  to be used in ${name}_lib
	else(ISIS_BUILD_STATIC_AND_SHARED)
		if(ISIS_BUILD_STATIC_LIBS)
			build_lib(${name} STATIC "${sources}" "${deps}" ${soversion} ${version})
		else(ISIS_BUILD_STATIC_LIBS)
			build_lib(${name} SHARED "${sources}" "${deps}" ${soversion} ${version})
		endif(ISIS_BUILD_STATIC_LIBS)
	endif(ISIS_BUILD_STATIC_AND_SHARED)
endmacro( add_lib )
				
add_subdirectory(Core)

add_subdirectory(ImageIO)
add_subdirectory(Adapter)

