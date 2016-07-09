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
# export library dependencies of isis_core
############################################################
foreach(FILE ${Boost_LIBRARIES})
		string(REGEX MATCH "${CMAKE_SHARED_LIBRARY_SUFFIX}$" IS_SHARED ${FILE})
		if(IS_SHARED)
				LIST(APPEND ISIS_MATH_LIB_DEPENDS ${FILE})
		endif(IS_SHARED)
endforeach(FILE)
if(ISIS_MATH_LIB_DEPENDS)
	message(STATUS "${CMAKE_SYSTEM_NAME} core dependencies are:  ${ISIS_MATH_LIB_DEPENDS}")
else(ISIS_MATH_LIB_DEPENDS)
	message(STATUS "no math dependencies detected")
endif(ISIS_MATH_LIB_DEPENDS)


############################################################
# build the core library
############################################################

# add source files to this parameter list
file(GLOB SRC_FILES "*.cpp")
file(GLOB HDR_FILES "*.hpp")

#add the core library
add_lib(isis_math "${SRC_FILES}" "${isis_core_lib};${Boost_LIBRARIES}" ${ISIS_CORE_VERSION_SO} ${ISIS_CORE_VERSION_API})

############################################################
# Installation
############################################################

# install header files
install(FILES ${HDR_FILES} DESTINATION include/isis/math COMPONENT Development)

# write configuration file
#configure_file(cmake/isis_mathcfg.cmake.in ${CMAKE_BINARY_DIR}/ISISMath.cmake @ONLY)

# install cmake configuration files
#install(FILES ${CMAKE_BINARY_DIR}/ISISConfig.cmake DESTINATION share/isis/cmake COMPONENT Development)

