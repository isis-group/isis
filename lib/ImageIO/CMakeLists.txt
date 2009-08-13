add_definitions(-D_ENABLE_DATA_LOG=1)

#list all source files here
add_library(imageformats SHARED ImageFormat ImageFormatFactory.cpp )

set_target_properties(imageformats PROPERTIES 
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
    LINK_FLAGS "--no-undefined")
