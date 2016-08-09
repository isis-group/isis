############################################################
# export library dependencies of isis_core
############################################################

LIST(APPEND _DEPENDS ${Boost_LIBRARIES})

set(_CPP_GLOB "*.cpp" "details/details_fft.cpp")
set(_HPP_GLOB "*.hpp")

option(ISIS_USE_GSL "use gsl for math functions like fft" OFF)
option(ISIS_USE_clFFT "use clFFT for fft" OFF)

if(ISIS_USE_GSL)
	if (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
		find_package(PkgConfig)
		pkg_search_module(GSL gsl)
		include_directories(GSL_INCLUDE_DIRS)
		LIST(APPEND _DEPENDS ${GSL_LIBRARIES})
	else(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
		find_package(GSL)
		include_directories(${GSL_INCLUDE_DIR})
		LIST(APPEND _DEPENDS ${GSL_LIBRARY})
	endif(${CMAKE_SYSTEM_NAME} MATCHES "Linux")

	LIST(APPEND _CPP_GLOB "gsl/*.cpp")
	LIST(APPEND _HPP_GLOB "gsl/*.hpp")

	add_definitions("-DHAVE_GSL")
endif(ISIS_USE_GSL)

if(ISIS_USE_clFFT)
	find_package(clFFT)
	list(APPEND _DEPENDS clFFT)
	LIST(APPEND _CPP_GLOB "details/opencl/*.cpp")
	LIST(APPEND _CPP_GLOB "details/clfft.cpp")
	add_definitions("-DHAVE_CLFFT")
endif(ISIS_USE_clFFT)

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
file(GLOB SRC_FILES ${_CPP_GLOB})
file(GLOB HDR_FILES ${_HPP_GLOB})

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
