add_definitions(-D_ENABLE_DATA_LOG=1)

#list all source files here
add_library(isisImageFormatDCM SHARED ImageFormatDCM.cpp)
add_library(isisImageFormatNii SHARED ImageFormatNii.cpp)

set_target_properties( PROPERTIES 
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
    LINK_FLAGS "--no-undefined")
