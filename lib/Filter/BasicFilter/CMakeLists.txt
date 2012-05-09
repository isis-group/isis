include_directories(${CMAKE_SOURCE_DIR}/lib/Core)

set(BASIC_FILTER 	GaussianFilter.cpp
					GaussianKernelFilter.cpp
					ConvolutionFilter.cpp)

add_library(isisFilter_Basic SHARED ${BASIC_FILTER})
target_link_libraries(isisFilter_Basic ${ISIS_LIB})

if(WIN32)
install(TARGETS isisFilter_Basic  RUNTIME DESTINATION bin COMPONENT "filter")
else(WIN32)
install(TARGETS isisFilter_Basic DESTINATION ${ISIS_FILTER_INFIX} COMPONENT "filter" )
endif(WIN32)