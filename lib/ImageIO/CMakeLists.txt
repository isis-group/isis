add_definitions(-D_ENABLE_DATA_LOG=1)

#list all source files here

add_library(isisImageFormatNii SHARED ImageFormatNii.cpp)
add_library(isisImageFormat_Null SHARED imageFormat_Null.cpp)

 
set_target_properties(isisImageFormatNii PROPERTIES 
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
    LINK_FLAGS "--no-undefined")

    