
add_library(extitk SHARED isisTransformMerger3D.cpp isisTransformMerger2D.cpp)

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