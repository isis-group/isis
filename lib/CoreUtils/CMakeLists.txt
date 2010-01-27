#list all source files here

add_library(utils type_base.cpp types.cpp property.cpp propmap.cpp message.cpp) #converter.cpp)
target_link_libraries(utils ${Boost_LIBRARIES})

set_target_properties(utils PROPERTIES 
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
    LINK_FLAGS "--no-undefined")

set_target_properties(utils PROPERTIES ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})