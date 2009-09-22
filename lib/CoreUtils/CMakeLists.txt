#list all source files here
add_library(utils SHARED types.cpp  propmap.cpp message.cpp)

set_target_properties(utils PROPERTIES 
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
    LINK_FLAGS "--no-undefined")
