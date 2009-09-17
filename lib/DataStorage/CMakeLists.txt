#list all source files here
add_library(data SHARED chunk.cpp ImageFormatFactory.cpp ImageFormat.cpp)

# needed for dlsym, dlopen, etc. pp.
target_link_libraries(data		dl)

set_target_properties(data PROPERTIES 
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
    LINK_FLAGS "--no-undefined")

