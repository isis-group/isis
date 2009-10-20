cmake_policy(SET CMP0005 OLD) #make it complain about escaped Preprocessor definition
add_definitions(-DDL_SUFFIX=\\"${CMAKE_SHARED_LIBRARY_SUFFIX}\\")
add_definitions(-DDL_PREFIX=\\"${CMAKE_SHARED_LIBRARY_PREFIX}\\")


if(BUILD_SHARED_LIBS)
    add_library(extitk SHARED isisTransformMerger.cpp)
else(BUILD_SHARED_LIBS)
    add_library(extitk STATIC isisTransformMerger.cpp)
ENDIF(BUILD_SHARED_LIBS)

set_target_properties(extitk PROPERTIES 
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
    LINK_FLAGS "--no-undefined")

set_target_properties(extitk PROPERTIES ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

FIND_PACKAGE(ITK)
IF(ITK_FOUND)
  INCLUDE(${ITK_USE_FILE})
ELSE(ITK_FOUND)
  MESSAGE(FATAL_ERROR
    "ITK not found. Please set ITK_DIR.")
ENDIF(ITK_FOUND)

#target_link_libraries(extITK ITKCommon)