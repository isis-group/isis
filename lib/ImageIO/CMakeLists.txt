add_library(isisImageFormat_Nifti SHARED imageFormat_Nifti.cpp)
add_library(isisImageFormat_Null SHARED imageFormat_Null.cpp)

target_link_libraries(isisImageFormat_Null utils data)
target_link_libraries(isisImageFormat_Nifti utils data)

set_target_properties(isisImageFormat_Nifti PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
    LINK_FLAGS "--no-undefined")

set_target_properties(isisImageFormat_Null PROPERTIES
	LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
	LINK_FLAGS "--no-undefined")

set_target_properties(isisImageFormat_Nifti PROPERTIES ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set_target_properties(isisImageFormat_Null PROPERTIES ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

message(STATUS "Installing plugins to ${CMAKE_BINARY_DIR}")

