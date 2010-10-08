FIND_PACKAGE(VTK REQUIRED)
INCLUDE(${USE_VTK_FILE})
add_library(isisAdapter_vtk SHARED vtkAdapter.cpp)
target_link_libraries(isisAdapter_vtk ${ISIS_LIB} )
set_target_properties(isisAdapter_vtk PROPERTIES LINK_FLAGS "--no-undefined" )
set_target_properties(isisAdapter_vtk PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
  INSTALL_RPATH_USE_LINK_PATH TRUE)
SET(TARGETS ${TARGETS} isisAdapter_vtk)