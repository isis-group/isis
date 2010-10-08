
# find the itk package
FIND_PACKAGE(ITK REQUIRED)
INCLUDE(${USE_ITK_FILE})

# this has to be done after INCLUDE(${USE_ITK_FILE}) because itk will overwrite this
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ftemplate-depth-100")

add_library(isisAdapter_itk SHARED itkAdapter.hpp)
set_target_properties(isisAdapter_itk PROPERTIES LINKER_LANGUAGE C)
set_target_properties(isisAdapter_itk PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
  INSTALL_RPATH_USE_LINK_PATH TRUE)
target_link_libraries(isisAdapter_itk ${ISIS_LIB} )
SET(TARGETS ${TARGETS} isisAdapter_itk)