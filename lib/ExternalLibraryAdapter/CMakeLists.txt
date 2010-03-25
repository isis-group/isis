cmake_minimum_required(VERSION 2.6)

IF(HAVE_VTK)
    include(${VTK_USE_FILE})
    add_library(vtkAdapter SHARED vtkAdapter.cpp)
    target_link_libraries(vtkAdapter utils data vtkImaging)
ENDIF(HAVE_VTK)