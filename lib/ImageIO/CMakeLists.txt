add_definitions(-D_ENABLE_DATA_LOG=1)

#list all source files here

add_library(isisImageFormat_Nifti SHARED imageFormat_Nifti.cpp)
add_library(isisImageFormat_Null SHARED imageFormat_Null.cpp)

 
set_target_properties(isisImageFormat_Nifti PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
    LINK_FLAGS "--no-undefined")

set_target_properties(isisImageFormat_Null PROPERTIES
	LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
	LINK_FLAGS "--no-undefined")

message(STATUS "Installing plugins to ${CMAKE_BINARY_DIR}")
