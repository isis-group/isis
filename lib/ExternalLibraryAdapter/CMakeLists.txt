
IF(ITK_FOUND)
	add_library(itkAdapter SHARED itkAdapter.hpp)
	# use add_library with hpp files as well
	set_target_properties(itkAdapter PROPERTIES LINKER_LANGUAGE C)
	target_link_libraries(itkAdapter utils data )
ENDIF(ITK_FOUND)

IF(VTK_FOUND)
	add_library(vtkAdapter SHARED vtkAdapter.cpp)
	target_link_libraries(vtkAdapter utils data )
ENDIF(VTK_FOUND)
