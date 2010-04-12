cmake_minimum_required(VERSION 2.6)


IF(ITK_FOUND)
	add_library(itkAdapter SHARED itkAdapter.cpp)
	target_link_libraries(itkAdapter utils data )
ENDIF(ITK_FOUND)

IF(VTK_FOUND)
	add_library(vtkAdapter SHARED vtkAdapter.cpp)
	target_link_libraries(vtkAdapter utils data )
ENDIF(VTK_FOUND)
