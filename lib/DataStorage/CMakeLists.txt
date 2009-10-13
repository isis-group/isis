cmake_policy(SET CMP0005 OLD) #make it complain about escaped Preprocessor definition
add_definitions(-DDL_SUFFIX=\\"${CMAKE_SHARED_LIBRARY_SUFFIX}\\")
add_definitions(-DDL_PREFIX=\\"${CMAKE_SHARED_LIBRARY_PREFIX}\\")

#list all source files here
IF(BUILD_SHARED_LIBS)
    add_library(data SHARED needed.cpp chunk.cpp propertyobject.cpp ndimensional.cpp image.cpp io_factory.cpp)
ELSE(BUILD_SHARED_LIBS)
    add_library(data STATIC needed.cpp chunk.cpp propertyobject.cpp ndimensional.cpp image.cpp io_factory.cpp)
ENDIF(BUILD_SHARED_LIBS)

# needed for dlsym, dlopen, etc. pp.
find_library(LIB_DL dl)

target_link_libraries(data utils ${LIB_DL} ${Boost_LIBRARIES})

set_target_properties(data PROPERTIES 
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
    LINK_FLAGS "--no-undefined")
    
set_target_properties(data PROPERTIES ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

