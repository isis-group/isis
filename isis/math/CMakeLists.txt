############################################################
# export library dependencies of isis_core
############################################################

LIST(APPEND _DEPENDS ${Boost_LIBRARIES})

set(_CPP_GLOB "*.cpp" "details/details_fft.cpp")
set(_HPP_GLOB "*.hpp")

option(ISIS_USE_GSL "use gsl for math functions like fft" OFF)
option(ISIS_USE_FFTW "use FFTW for fft" OFF)
option(ISIS_USE_clFFT "use clFFT for fft" OFF)

if(ISIS_USE_GSL)
	if (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
		find_package(PkgConfig)
		pkg_search_module(GSL REQUIRED gsl)
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

if(ISIS_USE_FFTW)
	if (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
		find_package(PkgConfig)
		pkg_search_module(FFTW REQUIRED fftw3)
		pkg_search_module(FFTWf REQUIRED fftw3f)
		include_directories(${FFTW_INCLUDE_DIRS} ${FFTWf_INCLUDE_DIRS})
		LIST(APPEND _DEPENDS ${FFTW_LIBRARIES} ${FFTWf_LIBRARIES})
	else(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
		message(FATAL_ERROR "Implement fftw lookup")
	endif(${CMAKE_SYSTEM_NAME} MATCHES "Linux")

	LIST(APPEND _CPP_GLOB "details/fftw.cpp")

	add_definitions("-DHAVE_FFTW")
endif(ISIS_USE_FFTW)


if(ISIS_USE_clFFT)
	find_package(clFFT)
	list(APPEND _DEPENDS clFFT)
	LIST(APPEND _CPP_GLOB "details/opencl/*.cpp")
	LIST(APPEND _CPP_GLOB "details/clfft.cpp")
	add_definitions("-DHAVE_CLFFT")
endif(ISIS_USE_clFFT)

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
configure_file(isis_mathcfg.cmake.in ${CMAKE_BINARY_DIR}/ISISMathConfig.cmake @ONLY)
install(FILES ${CMAKE_BINARY_DIR}/ISISMathConfig.cmake DESTINATION share/isis/cmake COMPONENT Development)
