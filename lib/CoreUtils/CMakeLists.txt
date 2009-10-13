#list all source files here
if(BUILD_SHARED_LIBS)
    add_library(utils SHARED vector.cpp type_base.cpp types.cpp property.cpp propmap.cpp message.cpp)
else(BUILD_SHARED_LIBS)
    add_library(utils STATIC vector.cpp type_base.cpp types.cpp property.cpp propmap.cpp message.cpp)
ENDIF(BUILD_SHARED_LIBS)

set_target_properties(utils PROPERTIES 
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
    LINK_FLAGS "--no-undefined")

set_target_properties(utils PROPERTIES ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})