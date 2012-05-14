include_directories(${CMAKE_SOURCE_DIR}/lib/Core)

set(BASIC_FILTER 	GaussianFilter.cpp
					GaussianKernelFilter.cpp
					ConvolutionFilter.cpp)

file(GLOB FILTER_HDR_FILES "*.hpp")

add_library(isisFilter_Basic SHARED ${BASIC_FILTER})
target_link_libraries(isisFilter_Basic isis_core)

if(WIN32)
install(TARGETS isisFilter_Basic  RUNTIME DESTINATION bin COMPONENT "filter")
install(FILES ${FILTER_HDR_FILES} DESTINATION include/isis/Filter COMPONENT Development )
else(WIN32)
install(TARGETS isisFilter_Basic DESTINATION ${ISIS_FILTER_INFIX} COMPONENT "filter" )
install(FILES ${FILTER_HDR_FILES} DESTINATION include/isis/Filter COMPONENT Development )
endif(WIN32)