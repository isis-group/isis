cmake_minimum_required(VERSION 2.6)


IF(HAVE_ITK)
	add_library(itkAdapter SHARED itkAdapter.cpp)
	target_link_libraries(itkAdapter utils data)
ENDIF(HAVE_ITK)

IF(HAVE_VTK)
	include(${VTK_USE_FILE})
	add_library(vtkAdapter SHARED vtkAdapter.cpp)
	target_link_libraries(vtkAdapter utils data vtkImaging)
ENDIF(HAVE_VTK)
