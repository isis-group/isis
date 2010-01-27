#list all source files here
add_library(data needed.cpp chunk.cpp ndimensional.cpp image.cpp io_factory.cpp io_interface.cpp)

target_link_libraries(data utils ${CMAKE_DL_LIBS} ${Boost_LIBRARIES})

set_target_properties(data PROPERTIES 
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
    LINK_FLAGS "--no-undefined")
    
set_target_properties(data PROPERTIES ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
