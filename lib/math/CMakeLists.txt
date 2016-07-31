############################################################
# export library dependencies of isis_core
############################################################

LIST(APPEND _DEPENDS ${Boost_LIBRARIES})

find_package(GSL)
include_directories(${GSL_INCLUDE_DIR})
LIST(APPEND _DEPENDS ${GSL_LIBRARY})


foreach(FILE ${_DEPENDS})
		string(REGEX MATCH "${CMAKE_SHARED_LIBRARY_SUFFIX}$" IS_SHARED ${FILE})
		if(IS_SHARED)
				LIST(APPEND ISIS_MATH_LIB_DEPENDS ${FILE})
		endif(IS_SHARED)
endforeach(FILE)
if(ISIS_MATH_LIB_DEPENDS)
	message(STATUS "${CMAKE_SYSTEM_NAME} math dependencies are:  ${ISIS_MATH_LIB_DEPENDS}")
else(ISIS_MATH_LIB_DEPENDS)
	message(STATUS "no math dependencies detected")
endif(ISIS_MATH_LIB_DEPENDS)


############################################################
# build the math library
############################################################

# add source files to this parameter list
file(GLOB SRC_FILES "*.cpp" "gsl/*.cpp")
file(GLOB HDR_FILES "*.hpp" "gsl/*.cpp")

#add the core library
add_lib(isis_math "${SRC_FILES}" "${isis_core_lib};${_DEPENDS}" ${ISIS_CORE_VERSION_SO} ${ISIS_CORE_VERSION_API})

############################################################
# Installation
############################################################

# install header files
install(FILES ${HDR_FILES} DESTINATION include/isis/math COMPONENT Development)

# write configuration file
#configure_file(cmake/isis_mathcfg.cmake.in ${CMAKE_BINARY_DIR}/ISISMath.cmake @ONLY)

# install cmake configuration files
#install(FILES ${CMAKE_BINARY_DIR}/ISISConfig.cmake DESTINATION share/isis/cmake COMPONENT Development)

